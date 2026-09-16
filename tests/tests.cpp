
#include <gtest/gtest.h>

#include "../console_app/faces_detector.h"

#include <filesystem>
#include <vector>

TEST(Detection, BaseDetectionTest)
{
    FaceDetector faces_detector(1);

    detector_context_t context = nullptr;
    ASSERT_NO_THROW(
        context = faces_detector.create_context()
    );

    EXPECT_NE(context, nullptr);

    auto const result_path = std::filesystem::path("image-1-result.jpg");

    std::vector<FaceRect> faces(1);
    int found_faces_count = 0;
    auto const result = faces_detector.detect_faces(
        context,
        std::filesystem::path("image-1.jpg"),
        result_path,
        faces,
        found_faces_count
    );

    EXPECT_EQ(result, DETECTION_OK);
    EXPECT_EQ(found_faces_count, 1);

    EXPECT_TRUE(std::filesystem::exists(result_path));

    ASSERT_NO_THROW(
        faces_detector.free_context(context)
    );
}
