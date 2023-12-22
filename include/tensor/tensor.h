#ifndef SIMPLE_BASE_TENSOR_H_
#define SIMPLE_BASE_TENSOR_H_

#include "common.h"
#include "log.h"
#include "manager/data_manager.h"

#include <algorithm>
#include <memory>
#include <new>
#include <string>
#include <vector>
namespace base {
#define TENSOR_SHAPE_MODE_NCHW ("NCHW")
#define TENSOR_SHAPE_MODE_NHWC ("NHWC")

class EXPORT_API Tensor final {
public:
    Tensor();

    ~Tensor() {}

    /// @brief Construct tensor
    /// @param[in] shape  : The shape of tensor
    /// @param[in] layout : The layout of tensor
    /// @param[in] mem_type : The memory type of tensor
    /// @param[in] element_type : The data type of tensor
    /// @note
    /// Now interface currently only supports fp32 data on the CPU
    /// layout can select NCHW or NHWC
    Tensor(const std::vector<uint32_t>& shape,
           const TensorLayout& layout,
           const MemoryType& mem_type,
           const DataType& element_type);

    /// @brief Construct tensor without  memory allocate and data copy
    /// @param[in] data_ptr  : The address of tensor for user
    /// @param[in] shape  : The shape of tensor
    /// @param[in] layout : The layout of tensor
    /// @param[in] mem_type : The memory type of tensor
    /// @param[in] element_type : The data type of tensor
    /// @note
    /// Now interface currently only supports fp32 data on the CPU
    /// layout can select NCHW or NHWC
    Tensor(const void* data_ptr,
           const std::vector<uint32_t>& shape,
           const TensorLayout& layout,
           const MemoryType& mem_type,
           const DataType& element_type);

    /// @brief Construct tensor without  memory allocate and data copy
    /// @param[in] data_mgr  : The data manager of tensor for user
    /// @param[in] shape  : The shape of tensor
    /// @param[in] layout : The layout of tensor
    /// @param[in] mem_type : The memory type of tensor
    /// @param[in] element_type : The data type of tensor
    /// @note
    /// Now interface currently only supports fp32 data on the CPU
    /// layout can select NCHW or NHWC
    Tensor(const std::shared_ptr<DataManager>& data_mgr,
           const std::vector<uint32_t>& shape,
           const TensorLayout& layout,
           const MemoryType& mem_type,
           const DataType& element_type);

    /// @brief Clone tensor, with data deep copy
    /// @param[out] replica : output Tensor.
    /// @note
    /// replica will take control of memory.
    /// the memory will not controled by raw tensor.
    /// User need to manager replica data
    std::shared_ptr<Tensor> Clone() const;

    /// @brief GetShape of tensor
    /// @note
    /// shape of tensor, Now only support shape.size() == 4
    inline std::vector<uint32_t> GetShape() const { return shape_; }

    /// @brief GetShape of tensor
    /// @note
    /// shape of tensor with index
    inline uint32_t GetShape(uint32_t idx) const {
        if (idx >= (uint32_t)shape_.size()) {
            SIMPLE_LOG_ERROR("index out of range, {} vs {}", idx, shape_.size());
            return -1;
        }
        return shape_[idx];
    }

    /// @brief GetStride of tensor
    /// @note
    /// stride of tensor is w * datetype
    /// as 1 * 3 * 224 * 224 with NCHW layout data type is float,
    /// stride = 224 * GetTypeSize()
    inline uint32_t GetStride() const { return stride_; }

    /// @brief GetElemType of tensor
    /// @note
    /// elemtype of tensor
    inline DataType GetElemType() const { return elem_type_; }

    /// @brief GetTypeSize of tensor
    /// @note
    /// type size of tensor
    /// as. M_DATA_TYPE_BYTE = 1
    /// M_DATA_TYPE_FLOAT32 = 4 etc
    inline uint32_t GetTypeSize() const { return type_size_; }

    /// @brief GetScalar of tensor
    /// @note
    /// scalar of tensor, with N = 1 of tensor total byte count
    /// as 4 * 3 * 224 * 224 with NCHW layout data type is float,
    /// scalar = 1 * 3 * 224 * 224 * GetTypeSize()
    inline uint32_t GetScalar() const { return nscalar_; }

    /// @brief GetSize of tensor
    /// @note
    /// size of tensor, tensor total byte count
    /// as 4 * 3 * 224 * 224 with NCHW layout data type is float,
    /// size = 4 * GetScalar() [3 * 224 * 224 * GetTypeSize()]
    inline uint32_t GetSize() const { return size_; }

    /// @brief GetCount of tensor
    /// @note
    /// count of tensor, tensor total byte count
    /// as 4 * 3 * 224 * 224 with NCHW layout data type is float,
    /// count = 4 * 3 * 224 * 224
    inline uint32_t GetCount() const { return size_ / type_size_; }

    /// @brief GetShapeMode of tensor
    /// @note
    /// layout of tensor, now only support NCHW or NHWC
    inline TensorLayout GetShapeMode() const { return shape_mode_; }

    /// @brief GetShapeModeStr of tensor
    /// @note
    /// layout of tensor, now only support NCHW or NHWC
    inline const std::string& GetShapeModeStr() const { return TensorLayoutStr[shape_mode_]; }

    /// @brief GetName of tensor
    /// @note
    /// name of tensor
    inline const std::string& GetName() const { return name_; }

    /// @brief GetDataManager of tensor
    /// @note
    /// data manager of tensor
    inline const std::shared_ptr<DataManager>& GetDataManager() const { return data_manager_; }

    /// @brief MemoryType of tensor
    /// @note
    /// memory type tensor
    inline const MemoryType& GetMemType() const { return data_manager_->GetMemType(); }

    /// @brief GetMemTypeStr of tensor
    /// @note
    /// memory type tensor of string
    inline const std::string& GetMemTypeStr() const { return MemTypeStr[mem_type_]; }

    /// @brief SetInitDone of tensor
    /// @note
    /// set init  tensor of flag
    inline void SetInitDone(const bool done) { init_done_ = done; }

    /// @brief SetName of tensor
    /// @note
    /// set name tensor of string
    inline void SetName(const std::string& name) { name_ = name; }

    /// @brief Get Address pointer if tensor
    /// @param[in] n  : The idx of tensor number.
    template <typename T>
    inline T* GetData(const size_t n = 0) const {
        if (shape_.size() == 0U || data_manager_ == nullptr) {
            return nullptr;
        }
        uint8_t* data = static_cast<uint8_t*>(data_manager_->GetDataPtr());
        void* v_data  = static_cast<void*>(data + n * nscalar_);
        return static_cast<T*>(v_data);
    }

    /// @brief GetDataAt, get tensor data with index offset value
    /// @param[in] offset offset
    /// @note is equal GetData<T>(0)[offset]
    template <typename T>
    inline T GetDataAt(const uint32_t offset = 0) {
        if (offset > GetCount() || data_manager_ == nullptr) {
            SIMPLE_LOG_ERROR("input error {} vs {}", offset, GetCount());
            return T(0);
        }
        T* data = static_cast<T*>(data_manager_->GetDataPtr());
        return data[offset];
    }
    bool operator==(const Tensor& other);
    bool operator!=(const Tensor& other);

private:
    MStatus CreatDataManager(const MemoryType& mem_type);
    MStatus InitImageParamters();

    std::vector<uint32_t> shape_;
    uint32_t stride_;
    uint32_t nscalar_;
    uint32_t size_;
    uint32_t type_size_;

    TensorLayout shape_mode_;
    DataType elem_type_;
    std::string name_;
    MemoryType mem_type_;
    std::shared_ptr<DataManager> data_manager_;

    bool use_cache_{false};
    bool init_done_{false};
};

inline std::string LogTensor(std::string prefix, const Tensor& tensor) {
    char ret[1024];
    sprintf(ret,
            "{%s} Tensor: Shape:[{%u},{%u},{%u},{%u}], Stride {%u}, Scalar {%u}, "
            "MemType {%s}, Layout {%s}, Data 0x{%lu}",
            prefix.c_str(),
            tensor.GetShape(0),
            tensor.GetShape(1),
            tensor.GetShape(2),
            tensor.GetShape(3),
            tensor.GetStride(),
            tensor.GetScalar(),
            tensor.GetMemTypeStr().c_str(),
            tensor.GetShapeModeStr().c_str(),
            reinterpret_cast<uint64_t>(tensor.GetData<uint8_t>(0)));
    return std::string(ret);
}

// Tensor API

/// @brief reshape tensor, and 2 Dims of shape as transpose
/// @param tensor input tensor of shape 4Dims
/// @return reshape of tensor
std::shared_ptr<Tensor> reshape(const std::shared_ptr<Tensor>& tensor);

/// @brief Transpose matrix operation of 2D
/// @param tensor input tensor of shape 2Dims
/// @return transpose of tensor, as swap rows and cols of matrix
/// @note now supports two dimensions
/// eg: {1, 1, rows, cols}-->{1, 1, cols, rows}
std::shared_ptr<Tensor> transpose(const std::shared_ptr<Tensor>& tensor);

/// @brief innerproduct tensor as left * right + bias
/// @param left left tensor
/// @param right right tensor
/// @param bias add bias of tensor
/// @return return left * right + bias
/// @note now supports two dimensions
std::shared_ptr<Tensor> innerproduct(const std::shared_ptr<Tensor>& left,
                                     const std::shared_ptr<Tensor>& right,
                                     const std::shared_ptr<Tensor>& bias);
} // namespace base
#endif // SIMPLE_BASE_TENSOR_H_