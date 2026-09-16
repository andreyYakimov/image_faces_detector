#pragma once

#include "api_header.h"

#include <memory>
#include <string_view>
#include <exception>


class FacesError : public std::exception
{
public:
	FacesError(int required_count):
		m_required_count(required_count)
	{
	}

	int required_count() const noexcept
	{
		return m_required_count;
	}

private:
	int const m_required_count;
};

class FaceDetector
{
public:
	static void set_detection_worker_count(int count);

public:
	FaceDetector(std::string_view model_path);
	~FaceDetector();

	void detect_image_faces(
		const char* image_path,
		const char* result_image_path,
		FaceRect* faces,
		int& faces_count);

private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};