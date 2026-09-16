#pragma once

#include "logger.h"

#include <mutex>

class ConsoleLogger : public ILogger
{
public:
	void info(std::string const& message) override;
	void warning(std::string const& message) override;
	void error(std::string const& message) override;

private:
	std::mutex m_mutex;
};