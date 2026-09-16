#include "api.h"

#include "face_detector.h"

#include <opencv2/core/utils/logger.hpp>

namespace
{

const char* FaceDetectionModelPath = "face_detection_yunet_2023mar.onnx";

}

int init_detector(int workers_count)
{
	try
	{
		FaceDetector::set_detection_worker_count(workers_count);

		cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	}
	catch (std::exception const&)
	{
		return static_cast<int>(DetectionOpResult::DETECTION_FAILURE);
	}
	catch (...)
	{
		return static_cast<int>(DetectionOpResult::DETECTION_UNKNOWN_FAILURE);
	}

	return static_cast<int>(DetectionOpResult::DETECTION_OK);
}

detector_context_t create_detection_context()
{
	try
	{
		return new FaceDetector(FaceDetectionModelPath);
	}
	catch (std::exception const&)
	{
	}
	catch (...)
	{
	}

	return 0;
}

void free_detection_context(detector_context_t context)
{
	try
	{
		if (context)
		{
			FaceDetector* detector = static_cast<FaceDetector*>(context);
			delete detector;
		}
	}
	catch (std::exception const&)
	{
	}
	catch (...)
	{
	}
}

int detect_image_faces(
	detector_context_t context,
	const char* image_path,
	const char* result_image_path,
	FaceRect* faces,
	int* faces_count,
	char* error_info,
	int* error_info_buffer_size
)
{
	try
	{
		if (!context || !image_path || !result_image_path || !faces_count)
			return static_cast<int>(DetectionOpResult::DETECTION_INVALID_ARGUMENT);

		if (*faces_count > 0 && !faces)
			return static_cast<int>(DetectionOpResult::DETECTION_INVALID_ARGUMENT);

		FaceDetector* detector = static_cast<FaceDetector*>(context);

		detector->detect_image_faces(image_path, result_image_path, faces, *faces_count);
	}
	catch (FacesError const& error)
	{
		*faces_count = error.required_count();
		return static_cast<int>(DetectionOpResult::DETECTION_FACES_BUFFER_TOO_SMALL);
	}
	catch (std::exception const& error)
	{
		if (error_info && !error_info_buffer_size)
			return static_cast<int>(DetectionOpResult::DETECTION_INVALID_ARGUMENT);

		// Save error info only if caller need it
		if (error_info && error_info_buffer_size)
		{
			std::string_view const what = error.what();
			if (*error_info_buffer_size < what.size() + 1)
			{
				*error_info_buffer_size = what.size() + 1;
				return static_cast<int>(DetectionOpResult::DETECTION_ERROR_BUFFER_TOO_SMALL);
			}

			std::copy(what.begin(), what.end(), error_info);
			error_info[what.size()] = '\0';
		}

		return static_cast<int>(DetectionOpResult::DETECTION_FAILURE);
	}
	catch (...)
	{
		return static_cast<int>(DetectionOpResult::DETECTION_UNKNOWN_FAILURE);
	}

	return static_cast<int>(DetectionOpResult::DETECTION_OK);
}
