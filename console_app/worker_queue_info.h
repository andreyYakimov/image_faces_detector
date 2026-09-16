#pragma once

#include "byte_limited_mpmc_queue.hpp"
#include "detection_result.h"

#include <filesystem>

// Approximate memory footprint.
struct PathSizePolicy
{
	std::size_t operator()(std::filesystem::path const& value) const noexcept
	{
		const auto& native = value.native();
		using NativeString = std::remove_cvref_t<decltype(native)>;

		return sizeof(value) + native.capacity() * sizeof(typename NativeString::value_type);
	}
};

struct DetectionResultSizePolicy
{
	std::size_t operator()(DetectionResult const& value) const noexcept
	{
		PathSizePolicy path_policy;

		return
			path_policy(value.original_image_path) +
			path_policy(value.result_image_path) +
			(value.faces.size() * sizeof(typename std::vector<FaceRect>::value_type));
	}
};

using WorkingQueueType = ByteLimitedMPMCQueue<std::filesystem::path, PathSizePolicy>;
using ResultQueueType = ByteLimitedMPMCQueue<DetectionResult, DetectionResultSizePolicy>;
