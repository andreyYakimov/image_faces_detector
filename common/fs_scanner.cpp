#include "fs_scanner.h"

#include "logger.h"

namespace
{

bool process_entry(
	std::filesystem::directory_entry const& entry,
	FileScanner::MasksType const& masks,
	ILogger& logger)
{
	try
	{
		if (!entry.is_regular_file())
			return false;

		const auto extension = entry.path().extension().string();

		return masks.contains(extension);
	}
	catch (std::filesystem::filesystem_error const& error)
	{
		logger.error(std::string("Filesystem error for '") + entry.path().string() + "': " + error.what());
	}
	catch (const std::exception& error)
	{
		logger.error(std::string("Error processing '") + entry.path().string() + "': " + error.what());
	}

	return false;
}

}

FileScanner::FileScanner(
	std::filesystem::path const& path,
	MasksType masks,
	ILogger& logger):
	m_path(path),
	m_masks(std::move(masks)),
	m_logger(logger)
{

}

void FileScanner::scan(FileScanner::Callback callback)
{
	try
	{
		auto rd_iterator = std::filesystem::recursive_directory_iterator(
			m_path,
			std::filesystem::directory_options::skip_permission_denied);

		for (const auto& entry : rd_iterator)
		{
			if (process_entry(entry, m_masks, m_logger))
			{
				callback(entry.path());
			}

			if (m_stopped)
			{
				break;
			}
		}
	}
	catch (const std::exception& error)
	{
		m_logger.error(std::string("Filesystem error while scanning '") + "': " + error.what());
	}
}

void FileScanner::stop() noexcept
{
	m_stopped.store(true);
}