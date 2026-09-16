cmake_minimum_required(VERSION 3.20)

project(ImageFaceDetector LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_subdirectory(common)
add_subdirectory(console_app)
add_subdirectory(detection_lib)
add_subdirectory(tests)

if(WIN32)
    set(RUNTIME_DEPENDENCY_EXCLUDE_REGEXES
        PRE_EXCLUDE_REGEXES
            "api-ms-"
            "ext-ms-"
        POST_EXCLUDE_REGEXES
            ".*system32/.*"
            ".*Windows/System32/.*"
    )
elseif(UNIX)
    set(RUNTIME_DEPENDENCY_EXCLUDE_REGEXES
        POST_EXCLUDE_REGEXES
            "^/lib/.*"
            "^/usr/lib/.*"
            "^/lib64/.*"
            "^/usr/lib64/.*"
    )
endif()

install(
    TARGETS
        ImageFaceDetector
        FaceDetector
    RUNTIME_DEPENDENCIES
        ${RUNTIME_DEPENDENCY_EXCLUDE_REGEXES}
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION .
)

install(
    TARGETS
        ImageFaceDetectorTests
    RUNTIME_DEPENDENCIES
        ${RUNTIME_DEPENDENCY_EXCLUDE_REGEXES}
    RUNTIME DESTINATION .
    LIBRARY DESTINATION .
    ARCHIVE DESTINATION .
)

install(
    FILES
        ${CMAKE_SOURCE_DIR}/model/face_detection_yunet_2023mar.onnx
        ${CMAKE_SOURCE_DIR}/tests/image-1.jpg
    DESTINATION .
)