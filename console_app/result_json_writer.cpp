#include "result_json_writer.h"

#include "logger.h"
#include "json/writer.h"

#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>


class ResultJsonWriter::Impl
{
public:
	Impl(std::filesystem::path path, ILogger& logger, std::size_t storage_capacity):
		m_path(std::move(path)),
		m_logger(logger),
		m_storage_capacity(storage_capacity),
		m_json_builder(),
		m_json_writer()
	{
		if (storage_capacity == 0)
		{
			throw std::invalid_argument("ResultJsonWriter: storage_capacity must be greater than zero");
		}

		prepare_out_file();
		prepare_json();

		m_storage.reserve(m_storage_capacity);
	}

	void prepare_json()
	{
		try
		{
			m_json_builder["indentation"] = "";
			m_json_writer.reset(m_json_builder.newStreamWriter());
		}
		catch (std::exception const& error)
		{
			m_logger.error(std::string("ResultJsonWriter: can't prepare json:") + error.what());
			throw;
		}
	}

	void prepare_out_file()
	{
		try
		{
			m_file.open(m_path, std::ios::binary | std::ios::trunc);
			if (!m_file)
			{
				throw std::ios_base::failure("Can't open result file: " + m_path.string());
			}

			m_file << "[\n";
		}
		catch (std::exception const& error)
		{
			m_logger.error(std::string("ResultJsonWriter: can't prepare file:") + error.what());
			throw;
		}
	}

	void final_write()
	{
		if (m_storage.empty())
			return;

		for (const auto& msg : m_storage)
		{
			if (!m_first)
				m_file << ",\n";

			m_file.write(
				reinterpret_cast<const char*>(msg.data()),
				static_cast<std::streamsize>(msg.size()));

			m_first = false;
		}
	}

	std::u8string process_result(DetectionResult const& sample)
	{
		Json::Value root;

		auto const image = sample.original_image_path.u8string();
		auto const result_image = sample.result_image_path.u8string();

		root["original_image_path"] = std::string(reinterpret_cast<const char*>(image.data()), image.size());
		root["result_image_path"] = std::string(reinterpret_cast<const char*>(result_image.data()), result_image.size());

		Json::Value faces(Json::arrayValue);
		for (const auto& face : sample.faces)
		{
			Json::Value rect(Json::arrayValue);

			rect.append(face.x);
			rect.append(face.y);
			rect.append(face.w);
			rect.append(face.h);

			faces.append(std::move(rect));
		}

		root["faces"] = std::move(faces);

		std::ostringstream output;
		m_json_writer->write(root, &output);

		const auto json = output.str();

		return std::u8string(reinterpret_cast<const char8_t*>(json.data()), json.size());
	}

	void write(DetectionResult const& sample)
	{
		try
		{
			m_storage.emplace_back(process_result(sample));

			if (m_storage.size() >= m_storage_capacity)
			{
				final_write();
				m_storage.clear();
			}
		}
		catch (std::exception const& error)
		{
			m_logger.error(
				std::string("ResultJsonWriter: can't write face detection info to result json: ") + error.what());
			throw;
		}
	}

	void flush()
	{
		if (!m_storage.empty())
		{
			final_write();
			m_storage.clear();
		}
	}

	void close()
	{
		if (!m_file.is_open())
			return;

		final_write();

		m_file << "\n]\n";

		m_file.close();
	}

private:
	const std::filesystem::path m_path;
	ILogger& m_logger;

	std::ofstream m_file;
	bool m_first = true;
	const std::size_t m_storage_capacity;
	std::vector<std::u8string> m_storage;
	Json::StreamWriterBuilder m_json_builder;
	std::unique_ptr<Json::StreamWriter> m_json_writer;
};


ResultJsonWriter::ResultJsonWriter(std::filesystem::path path, ILogger& logger, std::size_t storage_capacity):
	m_impl(std::make_unique<Impl>(std::move(path), logger, storage_capacity))
{

}

ResultJsonWriter::~ResultJsonWriter() = default;

void ResultJsonWriter::write(DetectionResult const& sample)
{
	m_impl->write(sample);
}

void ResultJsonWriter::close()
{
	m_impl->close();
}

void ResultJsonWriter::flush()
{
	m_impl->flush();
}