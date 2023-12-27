#include "common.h"
#include "log.h"
#include "manager/data_manager.h"
#include "tensor/tensor.h"
#include "utils/test_util.h"

#include <fstream>
#include <gtest/gtest.h>

class ManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef CONFIG_SIMPLE_BASE_ENABLE_SPDLOG
        close_level(true);
#endif
    }
};

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

TEST_F(TensorTest, Matrix_GetShape_API) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetShape(0), 1);
    EXPECT_EQ(tensor->GetShape(1), 1);
    EXPECT_EQ(tensor->GetShape(2), rows);
    EXPECT_EQ(tensor->GetShape(3), cols);
    auto matrix_shape = tensor->GetShape();
    EXPECT_TRUE(matrix_shape == shape);
}

TEST_F(TensorTest, Matrix_GetStride_API) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetStride(), cols * sizeof(float));
}

TEST_F(TensorTest, Matrix_GetElemType_API) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetElemType(), M_DATA_TYPE_FLOAT32);
}

TEST_F(TensorTest, Matrix_GetTypeSize_API) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetTypeSize(), sizeof(float));
}

TEST_F(TensorTest, Matrix_GetScalar_API) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{3, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetScalar(), rows * cols * sizeof(float));
}

TEST_F(TensorTest, Matrix_GetSize_API) {
    const int batch = 3, rows = 100, cols = 50, channel = 1;
    std::vector<uint32_t> shape{batch, channel, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetSize(), rows * cols * batch * channel * sizeof(float));
}

TEST_F(TensorTest, Matrix_GetCount_API) {
    const int batch = 3, rows = 100, cols = 50;
    std::vector<uint32_t> shape{batch, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetCount(), rows * cols * batch);
}

TEST_F(TensorTest, Matrix_GetShapeMode_API) {
    const int batch = 3, rows = 100, cols = 50;
    std::vector<uint32_t> shape{batch, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetShapeMode(), M_LAYOUT_NCHW);
    EXPECT_EQ(tensor->GetShapeModeStr(), TensorLayoutStr[tensor->GetShapeMode()]);
}

TEST_F(TensorTest, Matrix_GetDataManager_API) {
    const int batch = 3, rows = 100, cols = 50;
    std::vector<uint32_t> shape{batch, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_TRUE(tensor->GetDataManager() != nullptr);
}

TEST_F(TensorTest, Matrix_GetMemType_API) {
    const int batch = 3, rows = 100, cols = 50;
    std::vector<uint32_t> shape{batch, 1, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);

    EXPECT_EQ(tensor->GetMemType(), M_MEM_ON_CPU);
    EXPECT_EQ(tensor->GetMemTypeStr(), MemTypeStr[tensor->GetMemType()]);
}

TEST_F(TensorTest, Matrix_GetData_API) {
    const int batch = 3, rows = 4, cols = 5, channel = 1;
    std::vector<uint32_t> shape{batch, channel, rows, cols};
    auto tensor =
        std::make_shared<base::Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);

    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);
    EXPECT_TRUE(tensor->GetData<float>(0) != nullptr);

    float* head_ptr = tensor->GetData<float>(0);
    int offset      = rows * cols * channel;
    for (int i = 0; i < batch; i++) {
        float* batch_data = tensor->GetData<float>(i);
        EXPECT_EQ(batch_data, head_ptr + i * offset);
    }

    std::vector<float> nums(channel * rows * cols, 0);
    float val = 0.f;
    for (int i = 0; i < channel; i++) {
        for (int j = 0; j < rows; j++) {
            for (int k = 0; k < cols; ++k) {
                val += 1.f;
                nums[i * rows * cols + j * cols + k] = val;
            }
        }
    }

    // for (int i = 0; i < rows; i++) {
    //     for (int j = 0; j < cols; j++) {
    //         printf("%f   ", nums[i * cols + j]);
    //     }
    //     printf("\n");
    // }
    std::vector<uint32_t> nums_shape{1, channel, rows, cols};
    auto nums_tensor = std::make_shared<base::Tensor>(
        (void*)nums.data(), nums_shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);

    for (int i = 0; i < rows; i++) {
        float* row_ptr = nums_tensor->GetData<float>(0) + i * cols;
        for (int j = 0; j < cols; j++) {
            EXPECT_EQ(row_ptr[j], nums[i * cols + j]);
        }
    }
}

TEST_F(TensorTest, transpose) {
    const int rows = 100, cols = 50;
    std::vector<uint32_t> shape{1, 1, rows, cols};
    using namespace base;
    auto tensor = std::make_shared<Tensor>(shape, M_LAYOUT_NCHW, M_MEM_ON_CPU, M_DATA_TYPE_FLOAT32);
    ASSERT_NE(tensor, nullptr);
    init_random<float>(tensor->GetData<float>(0), rows * cols, 0, 1);
    auto tran_tensor = transpose(tensor);
    ASSERT_NE(tran_tensor, nullptr);
    EXPECT_TRUE(tran_tensor->GetShape(0) == tran_tensor->GetShape(1));
    EXPECT_TRUE(tran_tensor->GetShape(0) == 1);
    EXPECT_EQ(tensor->GetShape(2), tran_tensor->GetShape(3));
    EXPECT_EQ(tensor->GetShape(3), tran_tensor->GetShape(2));
}

TEST_F(ManagerTest, DataManager_API) {
    auto data_manager = std::make_shared<base::DataManager>();
    EXPECT_TRUE(data_manager != nullptr);
    EXPECT_EQ(data_manager->GetSize(), 0);

    void* data_ptr = data_manager->Malloc(1000);
    EXPECT_TRUE(data_ptr != nullptr);
    EXPECT_EQ(data_manager->GetSize(), 1000);
    EXPECT_EQ(data_ptr, data_manager->GetDataPtr());
}

TEST_F(ManagerTest, DataManager_Malloc) {
    for (int i = 0; i < 100; i++) {
        auto data_manager = std::make_shared<base::DataManager>();
        auto data         = data_manager->Malloc(1000);
        EXPECT_TRUE(((size_t)data & 1) == 0);
        EXPECT_TRUE(data != nullptr);
        data_manager->Free(data);
        EXPECT_TRUE(data_manager->GetDataPtr() == nullptr);
        EXPECT_EQ(data_manager->GetSize(), 0);
    }
}

TEST_F(ManagerTest, DataManager) {
    uint8_t* e_data = new uint8_t[1000];
    EXPECT_TRUE(e_data != nullptr);

    auto data_manager = std::make_shared<base::DataManager>();
    EXPECT_TRUE(data_manager != nullptr);

    auto set_data = data_manager->Setptr(e_data, 1000);
    EXPECT_TRUE(e_data == set_data);
    EXPECT_EQ(data_manager->GetSize(), 1000);
    EXPECT_EQ(data_manager->IsOwner(), false);
}