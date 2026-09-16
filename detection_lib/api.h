#pragma once

#include <stddef.h>

#include "api_header.h"

#if defined(_WIN32)
    #if defined(FACE_DETECTOR_BUILD)
        #define FACE_DETECTOR_API __declspec(dllexport)
    #else
        #define FACE_DETECTOR_API __declspec(dllimport)
    #endif
#else
    #define FACE_DETECTOR_API \
        __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

FACE_DETECTOR_API int init_detector(int workers_count);

FACE_DETECTOR_API detector_context_t create_detection_context();
FACE_DETECTOR_API void free_detection_context(detector_context_t context);

FACE_DETECTOR_API int detect_image_faces(
    detector_context_t context,
    const char* image_path,
    const char* result_image_path,
    FaceRect* faces,
    int* faces_count,
    char* error_info,
    int* error_info_buffer_size
    );


#ifdef __cplusplus
}
#endif
