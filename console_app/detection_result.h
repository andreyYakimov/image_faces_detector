#pragma once

#include "api_header.h"

#include <cstdint>
#include <filesystem>
#include <vector>

struct DetectionResult
{
	std::filesystem::path original_image_path;
	std::filesystem::path result_image_path;
	std::vector<FaceRect> faces;
};
