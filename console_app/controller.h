#pragma once

#include <memory>

#include <filesystem>

class ILogger;

class Controller
{
public:
	Controller(
		ILogger& logger,
		std::filesystem::path const& path,
		std::filesystem::path const& out_path,
		std::filesystem::path const& result_json_path,
		int detection_worker_count,
		int worker_count);
	~Controller();

	bool is_finished() const;

	void start_pipeline();
	void stop_pipeline();

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};