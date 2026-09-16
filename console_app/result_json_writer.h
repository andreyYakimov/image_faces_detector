#pragma once

#include "result_writer.h"

#include <string>
#include <filesystem>
#include <memory>

class ILogger;

class ResultJsonWriter : public IResultWriter
{
public:
	ResultJsonWriter(std::filesystem::path path, ILogger& logger, std::size_t storage_capacity);
	~ResultJsonWriter();

	void write(DetectionResult const& sample) override;
	void flush() override;
	void close() override;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};
