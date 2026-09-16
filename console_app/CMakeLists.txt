find_package(Boost CONFIG REQUIRED COMPONENTS
    program_options
    dll
)

find_package(jsoncpp CONFIG REQUIRED)

add_executable(ImageFaceDetector
    main.cpp
    result_writer.h
    result_json_writer.cpp
    result_json_writer.h
    result_console_writer.cpp
    result_console_writer.h
    result_collector.cpp
    result_collector.h
    file_scanner_worker.cpp
    file_scanner_worker.h
	controller.h
	controller.cpp
	faces_detector.h
	faces_detector.cpp
    detection_result.h
	worker_queue_info.h
)

target_include_directories(ImageFaceDetector PRIVATE
    ${CMAKE_SOURCE_DIR}/detection_lib
)


target_link_libraries(ImageFaceDetector
    PRIVATE
        JsonCpp::JsonCpp
)

target_link_libraries(ImageFaceDetector
    PRIVATE
        common
        Boost::program_options
        Boost::dll
)
