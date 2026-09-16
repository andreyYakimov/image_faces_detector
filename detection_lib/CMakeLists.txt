find_package(OpenCV CONFIG REQUIRED
    COMPONENTS
        core
        imgcodecs
        imgproc
        objdetect
)

add_library(FaceDetector SHARED
    face_detector.cpp
    face_detector.h
	api.h
	api.cpp
	api_header.h
)

target_include_directories(FaceDetector
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(FaceDetector
    PRIVATE
        FACE_DETECTOR_BUILD
)

target_link_libraries(FaceDetector
    PRIVATE
        common
        opencv_core
        opencv_imgcodecs
        opencv_imgproc
        opencv_objdetect
)
