#include <fstream>

#include "utils/test_util.h"
#include "gtest/gtest.h"

class ImageTest : public ::testing::Test {
protected:
    void SetUp() override {}
};

TEST_F(ImageTest, InvalidInput) {}

TEST(ImageTest, ValidInput) {}