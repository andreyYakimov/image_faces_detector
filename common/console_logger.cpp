#include "console_logger.h"

#include <iostream>

void ConsoleLogger::info(std::string const& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "[INFO] " << message << '\n';
}

void ConsoleLogger::warning(std::string const& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "[WARN] " << message << '\n';
}

void ConsoleLogger::error(std::string const& message)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::cout << "[ERROR] " << message << '\n';
    std::cerr << "[ERROR] " << message << '\n';
}
