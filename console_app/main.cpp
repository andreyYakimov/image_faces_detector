
#include "controller.h"

#include <boost/program_options.hpp>

#include "console_logger.h"

#include <atomic>
#include <csignal>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <thread>
#include <chrono>

namespace
{

std::atomic_bool is_stopped{false};

void signal_handler(int)
{
    is_stopped = true;
}

int cpu_count()
{
    const auto n = static_cast<int>(std::thread::hardware_concurrency());
    return n ? n : 1;
}

struct EmptyLogger : public ILogger
{
    void info(std::string const&) override {}
    void warning(std::string const&) override {}
    void error(std::string const&) override {}
};

std::unique_ptr<ILogger> create_logger(bool logging_enabled)
{
    if (logging_enabled)
        return std::make_unique<ConsoleLogger>();

    return std::make_unique<EmptyLogger>();
}

}

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	std::filesystem::path images_path;
	std::filesystem::path result_images_path;

    const int cpus = cpu_count();
    int cv_threads = cpus >= 8 ? 2 : 1;
    int workers = cpus;
    bool logging_enabled = false;

    po::options_description options("Options");

    try
    {
        options.add_options()
            ("images_path", po::value<std::filesystem::path>(&images_path)->required(),
                "Input images path")

            ("result_images_path", po::value<std::filesystem::path>(&result_images_path)->required(),
                "Result images path")

            ("cv_threads", po::value<int>(&cv_threads)->default_value(cv_threads),
                "OpenCV worker count")

            ("workers", po::value<int>(&workers)->default_value(workers),
                "Detection workers count")

            ("logging", po::value<bool>(&logging_enabled)->default_value(logging_enabled),
                "Enable logging to console");

        po::variables_map variables;

        po::store(po::parse_command_line(argc, argv, options), variables);
        po::notify(variables);
    }
    catch (const po::error& error)
    {
        std::cout << error.what() << '\n';
        std::cout << options << '\n';

        return 1;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    try
    {
        if (workers <= 0)
            throw std::invalid_argument("workers should at least 1");
        
        if (cv_threads <= 0)
            throw std::invalid_argument("cv_threads should at least 1");

        if (!std::filesystem::is_directory(images_path))
            throw std::invalid_argument("images_path is not a valid directory");

        if (!std::filesystem::is_directory(result_images_path))
            throw std::invalid_argument("result_images_path is not a valid directory");

        std::filesystem::path const result_json_path = images_path / "result.json";

        auto logger = create_logger(logging_enabled);

        Controller controller = Controller(
            *logger,
            images_path,
            result_images_path,
            result_json_path,
            cv_threads,
            workers);

        controller.start_pipeline();

        while (!controller.is_finished())
        {
            if (is_stopped.load())
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        controller.stop_pipeline();
    }
    catch (std::exception const& error)
    {
        std::cout << error.what() << '\n';
        return 1;
    }

    return 0;
}