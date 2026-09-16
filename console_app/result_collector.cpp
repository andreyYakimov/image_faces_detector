#include "result_collector.h"

#include <array>


DetectionResultCollector::DetectionResultCollector(
	SubscribersType subscribers,
	ResultQueueType& queue,
	DoneCallback callback):
	m_subscribers(std::move(subscribers)),
	m_queue(queue),
	m_done_callback(std::move(callback))
{
}

void DetectionResultCollector::run()
{
	try
	{
		std::array<DetectionResult, 1> result;
		while (m_queue.pop(result) > 0)
		{
			for (auto& subscriber : m_subscribers)
			{
				subscriber.get().write(result[0]);
			}
		}

		for (auto& subscriber : m_subscribers)
		{
			subscriber.get().flush();
			subscriber.get().close();
		}

		if (m_done_callback)
			m_done_callback(true);
	}
	catch (std::exception const&)
	{
		if (m_done_callback)
			m_done_callback(false);
	}
}

void DetectionResultCollector::stop()
{
	m_queue.stop();
}
