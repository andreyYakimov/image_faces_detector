#include "faces_detector.h"

#include "api.h"

#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>

namespace
{

boost::filesystem::path get_face_detector_lib()
{
	return boost::dll::shared_library::decorate("FaceDetector");
}

}

class FaceDetector::Impl
{
public:
	Impl(int worker_count):
		m_lib(get_face_detector_lib()),
		m_init(m_lib.get<InitFunctionType>("init_detector")),
		m_create(m_lib.get<CreateContextFunctionType>("create_detection_context")),
		m_free(m_lib.get<FreeContextFunctionType>("free_detection_context")),
		m_detect(m_lib.get<DetectFunctionType>("detect_image_faces"))
	{
		m_init(worker_count);
	}

	detector_context_t create_context()
	{
		return m_create();
	}

	void free_context(detector_context_t context)
	{
		m_free(context);
	}

	DetectionOpResult detect_faces(
		detector_context_t context,
		std::filesystem::path const& image_path,
		std::filesystem::path const& result_image_path,
		std::vector<FaceRect>& faces,
		int& found_faces_count)
	{
		found_faces_count = static_cast<int>(faces.size());

		auto const image_path_u8 = image_path.u8string();
		auto const result_image_path_u8 = result_image_path.u8string();

		const char* image_path_u8_raw = reinterpret_cast<const char*>(image_path_u8.c_str());
		const char* result_image_path_u8_raw = reinterpret_cast<const char*>(result_image_path_u8.c_str());

		return static_cast<DetectionOpResult>(m_detect(
			context,
			image_path_u8_raw,
			result_image_path_u8_raw,
			faces.data(),
			&found_faces_count,
			nullptr,
			nullptr));
	}

private:
	using InitFunctionType = decltype(init_detector);
	using CreateContextFunctionType = decltype(create_detection_context);
	using FreeContextFunctionType = decltype(free_detection_context);
	using DetectFunctionType = decltype(detect_image_faces);

private:
	boost::dll::shared_library m_lib;

	InitFunctionType* m_init;
	CreateContextFunctionType* m_create;
	FreeContextFunctionType* m_free;
	DetectFunctionType* m_detect;
};


FaceDetector::FaceDetector(int worker_count):
	m_impl(std::make_unique<Impl>(worker_count))
{
}

detector_context_t FaceDetector::create_context()
{
	return m_impl->create_context();
}

void FaceDetector::free_context(detector_context_t context)
{
	m_impl->free_context(context);
}

DetectionOpResult FaceDetector::detect_faces(
	detector_context_t context,
	std::filesystem::path const& image_path,
	std::filesystem::path const& result_image_path,
	std::vector<FaceRect>& faces,
	int& found_faces_count)
{
	return m_impl->detect_faces(
		context, image_path, result_image_path, faces, found_faces_count);
}

FaceDetector::~FaceDetector() = default;