find_package(GTest CONFIG REQUIRED)

find_package(Boost CONFIG REQUIRED COMPONENTS
    dll
)

add_executable(ImageFaceDetectorTests
    tests.cpp
	../console_app/faces_detector.cpp
)

target_link_libraries(ImageFaceDetectorTests
    PRIVATE
        FaceDetector
        GTest::gtest_main
		Boost::dll
)

target_include_directories(ImageFaceDetectorTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/console_app
)

include(GoogleTest)

gtest_discover_tests(ImageFaceDetectorTests)