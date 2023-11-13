#include "manager/data_manager.h"

namespace base {

void* DataManger::Malloc(const uint32_t size) {
    data_ = static_cast<uint8_t*>(fast_malloc(size));
    size_ = size;
    return data_;
}

void* DataManger::Setptr(void* ptr, uint32_t size) {
    if (ptr == nullptr || size == 0) {
        SIMPLE_LOG_ERROR("Setptr err {}", size);
        return nullptr;
    }

    data_ = static_cast<uint8_t*>(ptr);
    size_ = size;
    return data_;
}

void DataManger::Free() {
    if (!use_cache_) {
        fast_free(data_);
        size_ = 0U;
    }
}

std::shared_ptr<DataManger> DataManger::Create() const {
    return std::static_pointer_cast<DataManger>(std::make_shared<DataManger>());
}

MStatus DataManger::SyncCache(bool io) {
    return MStatus::M_OK;
}

} // namespace base