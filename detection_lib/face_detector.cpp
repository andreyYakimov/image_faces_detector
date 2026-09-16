#include "face_detector.h"

#include "api_header.h"

#include <opencv2/core.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <stdexcept>
#include <filesystem>
#include <vector>
#include <fstream>

class FaceDetector::Impl
{
public:
	Impl(std::string_view model_path)
	{
		m_detector = cv::FaceDetectorYN::create(
			std::string(model_path),
			"",
			cv::Size(320, 320),
			0.6f,
			0.3f,
			5000);

		if (m_detector.empty())
		{
			throw std::runtime_error(std::string("Can't create YuNet detector from model: ") + std::string(model_path));
		}
	}

	void find_faces(const cv::Mat& image, FaceRect* faces, int& faces_count)
	{
        if (image.empty())
            throw std::invalid_argument("Image is empty");

        m_detector->setInputSize(image.size());

        cv::Mat detections;

        if (!m_detector->detect(image, detections))
        {
            faces_count = 0;
            return;
        }

        if (detections.rows > faces_count)
        {
            throw FacesError(detections.rows);
        }

        for (int i = 0; i < detections.rows; ++i)
        {
            const float x = detections.at<float>(i, 0);
            const float y = detections.at<float>(i, 1);
            const float w = detections.at<float>(i, 2);
            const float h = detections.at<float>(i, 3);

            faces[i].x = static_cast<std::int32_t>(x);
            faces[i].y = static_cast<std::int32_t>(y);
            faces[i].w = static_cast<std::int32_t>(w);
            faces[i].h = static_cast<std::int32_t>(h);
        }

        faces_count = detections.rows;
	}

    void blur_faces(cv::Mat& image, const FaceRect* faces, const int& faces_count)
    {
        const cv::Rect image_bounds(
            0,
            0,
            image.cols,
            image.rows);

        for (int i = 0; i < faces_count; ++i)
        {
            const cv::Rect face_rect(
                faces[i].x,
                faces[i].y,
                faces[i].w,
                faces[i].h);

            const cv::Rect roi = face_rect & image_bounds;

            if (roi.empty())
                continue;

            cv::GaussianBlur(
                image(roi),
                image(roi),
                cv::Size(0, 0),
                35.0,
                35.0,
                cv::BORDER_DEFAULT
            );
        }
    }

    void resize_and_save_image(cv::Mat& image, const char* result_image_path)
    {
        cv::Mat resized;

        cv::resize(
            image,
            resized,
            cv::Size(image.cols / 2, image.rows / 2),
            0.0,
            0.0,
            cv::INTER_AREA);

        std::vector<std::uint8_t> data;

        if (!cv::imencode(
            ".jpg",
            resized,
            data,
            {
                cv::IMWRITE_JPEG_QUALITY,
                90
            }))
        {
            throw std::runtime_error("Can't encode image: " + std::string(result_image_path));
        }

        auto const u8_result_image_path = std::filesystem::u8path(result_image_path);

        std::ofstream file(u8_result_image_path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Can't open result image: " + std::string(result_image_path));
        }

        file.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size()));

        file.close();
    }

    void process_image(
        const char* image_path,
        const char* result_image_path,
        FaceRect* faces,
        int& faces_count)
    {
        auto const u8_image_path = std::filesystem::u8path(image_path);
        std::ifstream file(u8_image_path, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Can't read image: " + std::string(image_path));
        }

        std::vector<std::uint8_t> data(std::istreambuf_iterator<char>(file), {});

        cv::Mat image = cv::imdecode(data, cv::IMREAD_COLOR);

        if (image.empty())
        {
            throw std::runtime_error("Can't decode image: " + std::string(image_path));
        }

        find_faces(image, faces, faces_count);

        if (faces_count > 0)
        {
            blur_faces(image, faces, faces_count);
            resize_and_save_image(image, result_image_path);
        }
    }

private:
	cv::Ptr<cv::FaceDetectorYN> m_detector;
};


FaceDetector::FaceDetector(std::string_view model_path) :
	m_impl(std::make_unique<Impl>(model_path))
{
}

FaceDetector::~FaceDetector() = default;

void FaceDetector::detect_image_faces(
    const char* image_path,
    const char* result_image_path,
    FaceRect* faces,
    int& faces_count)
{
    m_impl->process_image(image_path, result_image_path, faces, faces_count);
}

void FaceDetector::set_detection_worker_count(int count)
{
	cv::setNumThreads(count);
}
