#include "tensor/tensor.h"

#include <string.h>

namespace base {

static uint32_t TypeSize(DataType elem_type) {
    uint32_t size = 0;
    switch (elem_type) {
        case M_DATA_TYPE_INT8:
        case M_DATA_TYPE_UINT8:
            size = 1U;
            break;

        case M_DATA_TYPE_INT16:
        case M_DATA_TYPE_UINT16:
        case M_DATA_TYPE_FLOAT16:
            size = 2U;
            break;

        case M_DATA_TYPE_INT32:
        case M_DATA_TYPE_UINT32:
        case M_DATA_TYPE_FLOAT32:
            size = 4U;
            break;
        default:
            SIMPLE_LOG_ERROR("can't support %i\n", static_cast<int>(elem_type));
            break;
    }
    return size;
}

Tensor::Tensor()
    : shape_{},
      stride_{0},
      nscalar_{0},
      size_{0},
      type_size_{0},
      shape_mode_{},
      elem_type_{},
      name_{""},
      mem_type_(M_MEM_ON_CPU),
      data_manager_{nullptr},
      use_cache_{false},
      init_done_{false} {}

Tensor::Tensor(const std::vector<uint32_t>& shape,
               const TensorLayout& layout,
               const MemoryType& mem_type,
               const DataType& element_type)
    : shape_{shape},
      stride_{0},
      nscalar_{0},
      size_{0},
      type_size_{TypeSize(element_type)},
      shape_mode_{layout},
      elem_type_{element_type},
      name_{""},
      data_manager_{nullptr},
      use_cache_{false},
      init_done_{false} {
    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct tensor failed, init tensor paramters failed\n");
        return;
    }
    if (this->CreatDataManager(mem_type) != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct tensor failed, init tensor manager failed\n");
        return;
    }
    this->data_manager_->Malloc(this->size_);
}

Tensor::Tensor(const void* data_ptr,
               const std::vector<uint32_t>& shape,
               const TensorLayout& layout,
               const MemoryType& mem_type,
               const DataType& element_type)
    : shape_{shape},
      stride_{0},
      nscalar_{0},
      size_{0},
      type_size_{TypeSize(element_type)},
      shape_mode_{layout},
      elem_type_{element_type},
      name_{""},
      data_manager_{nullptr},
      use_cache_{true},
      init_done_{false} {
    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct tensor failed, init tensor paramters failed\n");
        return;
    }
    if (this->CreatDataManager(mem_type) != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct tensor failed, init tensor manager failed\n");
        return;
    }
    this->data_manager_->Setptr(const_cast<void*>(data_ptr), this->size_);
}

Tensor::Tensor(const std::shared_ptr<DataManager>& data_mgr,
               const std::vector<uint32_t>& shape,
               const TensorLayout& layout,
               const MemoryType& mem_type,
               const DataType& element_type)
    : shape_(shape),
      stride_{0},
      nscalar_{0},
      size_{0},
      type_size_{TypeSize(element_type)},
      shape_mode_{layout},
      elem_type_(element_type),
      name_(""),
      mem_type_(mem_type),
      data_manager_(data_mgr),
      use_cache_{false},
      init_done_(false) {
    if (nullptr == data_manager_) {
        SIMPLE_LOG_DEBUG("construct tensor failed, input data manager nullptr\n");
        return;
    }
    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct tensor failed, init tensor paramters failed\n");
        return;
    }
}

MStatus Tensor::InitImageParamters() {
    int LAYOUT_W = -1;
    int LAYOUT_H = -1;
    int LAYOUT_C = -1;
    switch (shape_mode_) {
        case TensorLayout::M_LAYOUT_NCHW:
            LAYOUT_W = 3;
            LAYOUT_H = 2;
            LAYOUT_C = 1;
            break;
        case TensorLayout::M_LAYOUT_NHWC:
            LAYOUT_W = 2;
            LAYOUT_H = 1;
            LAYOUT_C = 3;
            break;
        default:
            SIMPLE_LOG_ERROR("can't support layout\n");
            return MStatus::M_NOT_SUPPORT;
    }
    int LAYOUT_N = 0;
    stride_      = shape_[LAYOUT_W] * this->GetTypeSize();
    nscalar_     = shape_[LAYOUT_C] * shape_[LAYOUT_H] * stride_;
    size_        = nscalar_ * shape_[LAYOUT_N];
    init_done_   = true;
    return MStatus::M_OK;
}


MStatus Tensor::CreatDataManager(const MemoryType& mem_type) {
    std::string mem_type_str = DataManager::MemTypeToMemTypeStr(mem_type);
    SIMPLE_LOG_DEBUG("Tensor::CreatDataManager %s\n", mem_type_str.c_str());

    if (nullptr == this->data_manager_) {
        this->data_manager_ = std::make_shared<DataManager>();
    }
    if (nullptr == this->data_manager_) {
        SIMPLE_LOG_ERROR("%s data manager is nullptr", mem_type_str.c_str());
        return MStatus::M_FAILED;
    }
    return MStatus::M_OK;
}

std::shared_ptr<Tensor> Tensor::Clone() const {
    auto replica = std::make_shared<Tensor>(
        this->shape_, this->shape_mode_, this->mem_type_, this->elem_type_);
    if (nullptr == replica) {
        SIMPLE_LOG_ERROR("clone tensot failed\n");
        return nullptr;
    }
    memcpy(this->GetData<void>(), replica->GetData<void>(), replica->GetSize());
    return replica;
}

bool Tensor::operator==(const Tensor& other) {
    if (this->shape_ != other.shape_ || this->shape_mode_ != other.shape_mode_ ||
        this->mem_type_ != other.mem_type_ || this->name_ != other.name_ ||
        this->elem_type_ != other.elem_type_ || this->init_done_ != other.init_done_ ||
        this->stride_ != other.stride_ || this->nscalar_ != other.nscalar_ ||
        this->size_ != other.size_ || this->type_size_ != other.type_size_ ||
        this->GetData<uint8_t>() != other.GetData<uint8_t>()) {
        return false;
    }
    return true;
}
bool Tensor::operator!=(const Tensor& other) {
    return (*this == other) ? false : true;
}

} // namespace base
