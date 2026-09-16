#pragma once

#include <cstddef>
#include <cstdint>

typedef void* detector_context_t;

enum DetectionOpResult
{
	DETECTION_OK = 0,
	DETECTION_INVALID_ARGUMENT = 1,
	DETECTION_FAILURE = 2,
	DETECTION_UNKNOWN_FAILURE = 3,
	DETECTION_FACES_BUFFER_TOO_SMALL = 4,
	DETECTION_ERROR_BUFFER_TOO_SMALL = 5
};

struct FaceRect
{
	std::int32_t x;
	std::int32_t y;
	std::int32_t w;
	std::int32_t h;
};

static_assert(sizeof(FaceRect) == 16);
static_assert(alignof(FaceRect) == 4);

static_assert(offsetof(FaceRect, x) == 0);
static_assert(offsetof(FaceRect, y) == 4);
static_assert(offsetof(FaceRect, w) == 8);
static_assert(offsetof(FaceRect, h) == 12);
