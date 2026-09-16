#include "worker_group.h"

#include "non_copyable.hpp"
#include "logger.h"

#include <cstddef>
#include <thread>
#include <stdexcept>
#include <utility>
#include <vector>

// Worker lifetime is controlled by the owner.
class WorkerGroup::Impl: public NonCopyable
{
public:
	Impl(int thread_count, Worker worker, ILogger& logger) :
		m_thread_count(thread_count),
		m_worker(std::move(worker)),
		m_logger(logger)
	{
		if (thread_count == 0)
		{
			throw std::invalid_argument("WorkerGroup requires at least one worker");
		}
	}

	~Impl()
	{
		join();
	}

	void activate()
	{
		if (m_started)
		{
			throw std::logic_error("WorkerGroup already started");
		}

		m_threads.reserve(m_thread_count);

		for (std::size_t t_index = 0; t_index < m_thread_count; ++t_index)
		{
			m_threads.emplace_back([this]()
			{
				try
				{
					m_worker();
				}
				catch (std::exception const& error)
				{
					m_logger.error(std::string("Unhandled error in WorkerGroup worker: ") + error.what());
				}
				catch (...)
				{
					m_logger.error(std::string("Unhandled unknown error in WorkerGroup worker"));
				}
			});
		}

		m_started = true;
	}

	void join()
	{
		for (auto& thread : m_threads)
		{
			if (thread.joinable())
			{
				thread.join();
			}
		}
	}

private:
	const int m_thread_count;
	Worker m_worker;
	ILogger& m_logger;

	std::vector<std::thread> m_threads;
	bool m_started = false;
};


WorkerGroup::WorkerGroup(int thread_count, Worker worker, ILogger& logger):
	m_impl(std::make_unique<Impl>(thread_count, std::move(worker), logger))
{
}

WorkerGroup::~WorkerGroup() = default;

void WorkerGroup::activate()
{
	m_impl->activate();
}

void WorkerGroup::join()
{
	m_impl->join();
}
