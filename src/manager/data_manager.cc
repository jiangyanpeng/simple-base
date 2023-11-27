#include "manager/data_manager.h"

namespace base {

void* DataManager::Malloc(const uint32_t size) {
    data_ = static_cast<uint8_t*>(fast_malloc(size));
    size_ = size;
    return data_;
}

void* DataManager::Setptr(void* ptr, uint32_t size) {
    if (ptr == nullptr || size == 0) {
        SIMPLE_LOG_ERROR("Setptr err {}", size);
        return nullptr;
    }

    data_ = static_cast<uint8_t*>(ptr);
    size_ = size;
    return data_;
}

void DataManager::Free() {
    if (!use_cache_) {
        fast_free(data_);
        size_ = 0U;
    }
}

std::shared_ptr<DataManager> DataManager::Create() const {
    return std::static_pointer_cast<DataManager>(std::make_shared<DataManager>());
}

MStatus DataManager::SyncCache(bool io) {
    UNUSED_WARN(io);
    return MStatus::M_OK;
}

} // namespace base