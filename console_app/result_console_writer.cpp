#include "result_console_writer.h"

#include <sstream>
#include <string>
#include <iostream>

class ResultConsoleWriter::Impl
{
public:
	Impl(ILogger& logger):
		m_logger(logger)
	{

	}

	void write(DetectionResult const& sample)
	{
		auto const u8_original_image_path = sample.original_image_path.u8string();
		auto const u8_result_image_path = sample.result_image_path.u8string();

		auto const u8_original_image_path_string = std::string(
			reinterpret_cast<const char*>(u8_original_image_path.data()), u8_original_image_path.size()
		);
		auto const u8_result_image_path_string = std::string(
			reinterpret_cast<const char*>(u8_result_image_path.data()), u8_result_image_path.size()
		);

		std::ostringstream stream;
		stream
			<< "Original image path: " << u8_original_image_path_string << "\n"
			<< "Result image path: " << u8_result_image_path_string << "\n"
			<< "Faces: " << sample.faces.size() << "\n";

		std::cout << stream.str() << "\n";
	}

	void close()
	{
		// This is console. Nothing to close
	}

	void flush()
	{
		// This is console. Nothing to close
	}

private:
	ILogger& m_logger;
};


ResultConsoleWriter::ResultConsoleWriter(ILogger& logger):
	m_impl(std::make_unique<Impl>(logger))
{
}

ResultConsoleWriter::~ResultConsoleWriter() = default;

void ResultConsoleWriter::write(DetectionResult const& sample)
{
	m_impl->write(sample);
}

void ResultConsoleWriter::close()
{
	m_impl->close();
}

void ResultConsoleWriter::flush()
{
	m_impl->flush();
}