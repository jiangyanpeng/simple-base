#include "common.h"
#include "log.h"
#include "tensor/tensor.h"
#include "utils/test_util.h"

#include <fstream>
#include <gtest/gtest.h>

class ImageTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef CONFIG_SIMPLE_BASE_ENABLE_SPDLOG
        close_level();
#endif
    }
};

class TensorTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef CONFIG_SIMPLE_BASE_ENABLE_SPDLOG
        close_level();
#endif
    }
};

TEST_F(ImageTest, InvalidInput) {}

TEST_F(TensorTest, transpose) {
    const int rows = 10, cols = 20;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    using namespace base;
    auto tensor = std::make_shared<Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 100);
    auto tran_tensor = transpose(tensor);
    ASSERT_NE(tran_tensor, nullptr);
    EXPECT_TRUE(tran_tensor->GetShape(0) == tran_tensor->GetShape(1));
    EXPECT_TRUE(tran_tensor->GetShape(0) == 1);
    EXPECT_EQ(tensor->GetShape(2), tran_tensor->GetShape(3));
    EXPECT_EQ(tensor->GetShape(3), tran_tensor->GetShape(2));

    // auto se_tensor = transpose(tran_tensor);
    // for (int i = 0; i < rows; i++) {
    //     for (int j = 0; j < cols; j++) {
    //         EXPECT_NEAR(tensor->GetData<float>(0)[i * rows + j],
    //                     se_tensor->GetData<float>(0)[i * rows + j],
    //                     0.01);
    //     }
    // }
}