#include "controller.h"

#include "logger.h"
#include "byte_limited_mpmc_queue.hpp"
#include "worker_queue_info.h"
#include "file_scanner_worker.h"
#include "worker_group.h"
#include "result_collector.h"
#include "result_console_writer.h"
#include "result_json_writer.h"
#include "faces_detector.h"

#include <atomic>
#include <array>
#include <cstdint>
#include <string>
#include <mutex>
#include <sstream>

namespace
{

FileScanner::MasksType create_masks()
{
	return FileScanner::MasksType{
		".jpg",
		".bmp",
		".png",
		".webp",
		".tiff"
	};
}

constexpr int ResultQueuePushBatchSize = 4;
constexpr int FacesRectSize = 1000;

constexpr int MaxWorkingBufferSize = 10 * 1024 * 1024;
constexpr int MaxResultBufferSize = 10 * 1024 * 1024;

std::u8string generate_prefix()
{
	static std::atomic<std::uint64_t> counter{ 0 };

	const auto value = std::to_string(counter.fetch_add(1));

	return std::u8string(
		reinterpret_cast<const char8_t*>(value.data()),
		value.size());
}

std::u8string generate_image_name(std::u8string const& name)
{
	return name + u8"-" + generate_prefix();
}

std::filesystem::path generate_out_image_path(
	std::filesystem::path const& image_path, std::filesystem::path const& out_folder_path)
{
	auto const new_filename = generate_image_name(image_path.stem().u8string());
	auto const final_path = out_folder_path / (new_filename + image_path.extension().u8string());

	return final_path;
}

enum class ControllerState
{
	NotActivated,
	Initializing,
	Working,
	Stopping,
	Stopped
};

}

class Controller::Impl
{

public:
	Impl(ILogger& logger,
		std::filesystem::path const& path,
		std::filesystem::path const& out_path,
		std::filesystem::path const& result_json_path,
		int detection_worker_count,
		int worker_count):
		m_faceDetector(detection_worker_count),
		m_working_queue(MaxWorkingBufferSize),
		m_result_queue(MaxResultBufferSize),
		m_fs_scanner(),
		m_workers(),
		m_result_collector(),
		m_result_console_writer(),
		m_result_json_writer(),
		m_worker_count(worker_count),
		m_image_folder_path(path),
		m_out_image_folder_path(out_path),
		m_result_json_path(result_json_path),
		m_logger(logger)
	{
	}

	void initialize()
	{
		m_state.store(ControllerState::Initializing);

		m_fs_scanner = std::move(std::make_unique<FileScannerWorker>(
			m_image_folder_path,
			std::move(create_masks()),
			m_logger,
			m_working_queue,
			[this](bool processing_result, size_t images_count)
			{
				file_scanner_done(processing_result, images_count);
			}
		));

		m_result_console_writer = std::move(std::make_unique<ResultConsoleWriter>(m_logger));
		m_result_json_writer = std::move(std::make_unique<ResultJsonWriter>(
			m_result_json_path,
			m_logger,
			10
		));

		m_result_collector = std::move(std::make_unique<DetectionResultCollector>(
			DetectionResultCollector::SubscribersType{
				*m_result_console_writer,
				*m_result_json_writer },
				m_result_queue,
				[this](bool processing_result)
			{
				result_processing_done(processing_result);
			}
		));

		m_workers = std::move(std::make_unique<WorkerGroup>(
			m_worker_count,
			[this]()
			{
				detection_worker();
			},
			m_logger
		));
	}

	void destroy()
	{
		m_workers.reset();
		
		m_result_collector.reset();

		m_result_json_writer.reset();
		m_result_console_writer.reset();

		m_fs_scanner.reset();
	}

	void activate()
	{
		m_state.store(ControllerState::Working);

		m_result_collector->start();
		m_workers->activate();
		m_fs_scanner->start();
	}

	void deactivate()
	{
		m_fs_scanner->stop();
		m_result_collector->stop();

		m_fs_scanner->join();
		m_workers->join();
		m_result_collector->join();
	}

	bool detection_worker_finised()
	{
		std::lock_guard<std::mutex> lock(m_workers_mutex);
		return ++m_finished_workers_count == m_worker_count;
	}

	void detection_worker_handler()
	{
		std::unique_ptr<void, std::function<void(detector_context_t)>> context_lok(
			m_faceDetector.create_context(),
			[this](detector_context_t context)
			{
				if (context)
					m_faceDetector.free_context(context);
			}
		);

		if (!context_lok)
		{
			detection_worker_finised();
			detection_workers_done();
			return;
		}

		std::vector<FaceRect> faces(FacesRectSize);
		std::array<std::filesystem::path, ResultQueuePushBatchSize> buffer;

		while (auto count = m_working_queue.pop(buffer))
		{
			std::span const buffer_span(buffer.data(), count);
			for (auto const& image_path : buffer_span)
			{
				if (m_state.load() == ControllerState::Stopping)
					break;

				auto result = detection_worker_image_handler(
					context_lok.get(), image_path, m_out_image_folder_path, faces);

				if (result == DETECTION_FACES_BUFFER_TOO_SMALL)
				{
					result = detection_worker_image_handler(
						context_lok.get(), image_path, m_out_image_folder_path, faces);

					if (result == DETECTION_FACES_BUFFER_TOO_SMALL)
						throw std::runtime_error("double faces buffer to small error");
				}
			}
		}

		context_lok.reset();

		if (detection_worker_finised())
			detection_workers_done();
	}

	DetectionOpResult detection_worker_image_handler(
		detector_context_t context,
		std::filesystem::path const& image_path,
		std::filesystem::path const& out_image_foler_path,
		std::vector<FaceRect>& faces)
	{
		auto const out_image_path = generate_out_image_path(image_path, out_image_foler_path);

		int found_faces_count = 0;
		auto const detection_result = m_faceDetector.detect_faces(
			context, image_path, out_image_path, faces, found_faces_count);

		check_detection_result(detection_result);

		if (detection_result == DETECTION_FACES_BUFFER_TOO_SMALL)
		{
			faces.resize(found_faces_count);

			return DETECTION_FACES_BUFFER_TOO_SMALL;
		}

		if (detection_result == DETECTION_OK)
		{
			std::vector<FaceRect> found_faces;
			found_faces.resize(found_faces_count);
			std::move(faces.begin(), faces.begin() + found_faces_count, found_faces.begin());

			std::array<DetectionResult, 1> detection_result_buffer;
			detection_result_buffer[0] = DetectionResult{
				image_path,
				out_image_path,
				std::move(found_faces)
			};

			m_result_queue.push(detection_result_buffer);

			++m_processed_images_count;
			if (found_faces_count)
				++m_faced_images_count;
		}

		if (detection_result == DETECTION_FAILURE)
		{
			++m_failed_images_count;
		}

		return detection_result;
	}

	void detection_worker()
	{
		try
		{
			detection_worker_handler();
		}
		catch (std::exception const& error)
		{
			m_logger.error(std::string("Controller error: ") + error.what());

			mark_as_finished();
		}
	}

	void check_detection_result(DetectionOpResult result)
	{
		bool const failure =
			result == DETECTION_INVALID_ARGUMENT ||
			result == DETECTION_UNKNOWN_FAILURE;

		if (failure)
		{
			mark_as_finished();
		}
	}

	void file_scanner_done(bool result, size_t images_count)
	{
		m_images_count = images_count;
		m_scanner_done = true;

		if (!result)
		{
			mark_as_finished();
			return;
		}

		m_working_queue.stop();

		stop_pipeline_if_done();
	}

	void detection_workers_done()
	{
		m_detectors_done = true;

		m_result_queue.stop();

		stop_pipeline_if_done();
	}

	void result_processing_done(bool result)
	{
		m_result_processing_done = true;

		if (!result)
		{
			mark_as_finished();
			return;
		}

		stop_pipeline_if_done();
	}

	bool all_components_done() const
	{
		return m_scanner_done && m_result_processing_done && m_detectors_done;
	}

	void stop_pipeline_if_done()
	{
		if (all_components_done())
		{
			mark_as_finished();
		}
	}

	~Impl()
	{
	}

	void mark_as_finished()
	{
		m_state.store(ControllerState::Stopping);
	}

	void start_pipeline()
	{
		if (m_state.load() != ControllerState::NotActivated)
		{
			throw std::runtime_error("Controller is in an invalid state");
		}

		initialize();
		activate();
	}

	void log_results()
	{
		std::ostringstream stream;
		stream
			<< "Total images found: " << m_images_count << "\n"
			<< "Total images processed: " << m_processed_images_count << "\n"
			<< "Images with faces: " << m_faced_images_count << "\n"
			<< "Images with errors: " << m_failed_images_count << "\n";

		m_logger.info(stream.str());
	}

	void stop_pipeline()
	{
		if (m_state.load() == ControllerState::Stopped)
		{
			throw std::runtime_error("Controller is in an invalid state");
		}

		mark_as_finished();

		deactivate();
		destroy();

		log_results();

		m_state.store(ControllerState::Stopped);
	}

	bool is_finished() const
	{
		return m_state.load() == ControllerState::Stopping;
	}

private:
	ILogger& m_logger;
	FaceDetector m_faceDetector;
	WorkingQueueType m_working_queue;
	ResultQueueType m_result_queue;
	std::unique_ptr<FileScannerWorker> m_fs_scanner;
	std::unique_ptr<WorkerGroup> m_workers;
	std::unique_ptr<DetectionResultCollector> m_result_collector;
	std::unique_ptr<ResultConsoleWriter> m_result_console_writer;
	std::unique_ptr<ResultJsonWriter> m_result_json_writer;

	std::atomic_bool m_scanner_done = false;
	std::atomic_bool m_result_processing_done = false;
	std::atomic_bool m_detectors_done = false;

	std::atomic<size_t> m_images_count = 0;
	std::atomic<size_t> m_processed_images_count = 0;
	std::atomic<size_t> m_faced_images_count = 0;
	std::atomic<size_t> m_failed_images_count = 0;

	std::mutex m_workers_mutex;
	int m_finished_workers_count = 0;
	int m_worker_count;

	std::filesystem::path const m_image_folder_path;
	std::filesystem::path const m_out_image_folder_path;
	std::filesystem::path const m_result_json_path;

	std::atomic<ControllerState> m_state{ ControllerState::NotActivated };
};

Controller::Controller(
	ILogger& logger,
	std::filesystem::path const& path,
	std::filesystem::path const& out_path,
	std::filesystem::path const& result_json_path,
	int detection_worker_count,
	int worker_count):
	m_impl(std::make_unique<Impl>(
		logger, path, out_path, result_json_path, detection_worker_count, worker_count))
{
}

Controller::~Controller() = default;

bool Controller::is_finished() const
{
	return m_impl->is_finished();
}

void Controller::start_pipeline()
{
	m_impl->start_pipeline();
}

void Controller::stop_pipeline()
{
	m_impl->stop_pipeline();
}
