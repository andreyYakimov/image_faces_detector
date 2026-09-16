#pragma once

#include "api_header.h"

#include <memory>
#include <vector>
#include <filesystem>

class FaceDetector
{
public:
	FaceDetector(int worker_count);
	~FaceDetector();

	detector_context_t create_context();
	void free_context(detector_context_t context);

	DetectionOpResult detect_faces(
		detector_context_t context,
		std::filesystem::path const& image_path,
		std::filesystem::path const& result_image_path,
		std::vector<FaceRect>& faces,
		int& found_faces_count);

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};