#include "file_scanner_worker.h"

#include <stdexcept>

namespace
{

constexpr std::size_t MaxStorageSize = 10;

}


FileScannerWorker::FileScannerWorker(
	std::filesystem::path const& path,
	FileScanner::MasksType masks,
	ILogger& logger,
	WorkingQueueType& working_queue,
	DoneCallback done_callback):
	m_scanner(path, std::move(masks), logger),
	m_logger(logger),
	m_staging_queue(),
	m_working_queue(working_queue),
	m_done_callback(std::move(done_callback))
{
	if (MaxStorageSize > m_working_queue.capacity_bytes())
	{
		throw std::invalid_argument("Staging queue size more than working queue capacity.");
	}

	m_staging_queue.reserve(MaxStorageSize);
}

void FileScannerWorker::stop()
{
	m_working_queue.stop();
	m_scanner.stop();
}

void FileScannerWorker::run()
{
	try
	{
		m_scanner.scan([this](const std::filesystem::path& path)
		{
			try
			{
				++m_count;

				auto full_path = std::filesystem::absolute(path);

				m_staging_queue.push_back(std::move(full_path));
				if (m_staging_queue.size() >= MaxStorageSize)
				{
					if (!m_working_queue.push(m_staging_queue))
					{
						return;
					}

					m_staging_queue.clear();
				}
			}
			catch (std::exception const& error)
			{
				m_logger.error(std::string("FileScannerWorker: can't process path: ") + error.what());
			}
		});

		if (!m_staging_queue.empty())
		{
			m_working_queue.push(m_staging_queue);
		}

		m_done_callback(true, m_count);
	}
	catch (std::exception const& error)
	{
		m_logger.error(std::string("Error in FileScannerWorker: ") + error.what());

		m_done_callback(false, m_count);
	}
}