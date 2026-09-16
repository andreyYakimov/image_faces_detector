#pragma once

#include "runnable.h"
#include "logger.h"
#include "fs_scanner.h"
#include "byte_limited_mpmc_queue.hpp"
#include "worker_queue_info.h"

#include <filesystem>
#include <string>
#include <vector>

class FileScannerWorker : public Runnable
{
public:
	using StagingQueueType = std::vector<std::filesystem::path>;
	using DoneCallback = std::function<void(bool, size_t)>;

public:
	FileScannerWorker(
		std::filesystem::path const& path,
		FileScanner::MasksType masks,
		ILogger& logger,
		WorkingQueueType& working_queue,
		DoneCallback done_callback);

	void stop();

private:
	void run() override;

private:
	FileScanner m_scanner;
	ILogger& m_logger;
	StagingQueueType m_staging_queue;
	WorkingQueueType& m_working_queue;
	DoneCallback m_done_callback;
	size_t m_count{ 0 };
};
