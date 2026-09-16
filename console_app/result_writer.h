#pragma once

#include "detection_result.h"

#include <string>

class IResultWriter
{
public:
	virtual ~IResultWriter() = default;

	virtual void write(DetectionResult const&) = 0;
	virtual void close() = 0;
	virtual void flush() = 0;
};