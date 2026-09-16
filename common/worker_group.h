#pragma once

#include <functional>
#include <memory>

class ILogger;

class WorkerGroup
{
public:
	using Worker = std::function<void()>;

public:
	WorkerGroup(int thread_count, Worker worker, ILogger& logger);
	~WorkerGroup();

	void activate();
	void join();

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};