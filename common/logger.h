#pragma once

#include <string>

class ILogger
{
public:
	virtual ~ILogger() = default;

	virtual void info(std::string const& message) = 0;
	virtual void warning(std::string const& message) = 0;
	virtual void error(std::string const& message) = 0;
};
