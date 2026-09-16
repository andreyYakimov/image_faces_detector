#pragma once

#include "result_writer.h"

#include <memory>

class ILogger;

class ResultConsoleWriter : public IResultWriter
{
public:
	ResultConsoleWriter(ILogger& logger);
	~ResultConsoleWriter();

	void write(DetectionResult const& sample) override;
	void close() override;
	void flush() override;

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};