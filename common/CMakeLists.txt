add_library(common STATIC
	worker_group.h
	worker_group.cpp
	runnable.h
	runnable.cpp
	fs_scanner.h
	fs_scanner.cpp
	console_logger.cpp
    console_logger.h
	non_copyable.hpp
	logger.h
	byte_limited_mpmc_queue.hpp
)

target_include_directories(common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
