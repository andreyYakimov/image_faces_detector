#pragma once

#include "runnable.h"

#include "result_writer.h"
#include "byte_limited_mpmc_queue.hpp"
#include "detection_result.h"
#include "worker_queue_info.h"

#include <functional>
#include <vector>

class DetectionResultCollector: public Runnable
{
public:
	using SubscribersType = std::vector<std::reference_wrapper<IResultWriter>>;
	using DoneCallback = std::function<void(bool)>;

public:
	DetectionResultCollector(
		SubscribersType subscribers,
		ResultQueueType& queue,
		DoneCallback callback);

	void stop();

private:
	void run() override;

private:
	SubscribersType m_subscribers;
	ResultQueueType& m_queue;
	DoneCallback m_done_callback;
};