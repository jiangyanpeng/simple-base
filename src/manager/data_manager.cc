#include "manager/data_manager.h"

#include <list>
#include <mutex>
#include <sstream>
#include <vector>
namespace base {

void* DataManager::Malloc(const uint32_t size) {
    data_ = static_cast<uint8_t*>(fast_malloc(size));
    size_ = size;
    return data_;
}

void* DataManager::Setptr(void* ptr, uint32_t size) {
    if (ptr == nullptr || size == 0) {
        SIMPLE_LOG_ERROR("Setptr err %i", size);
        return nullptr;
    }
    SetOwer(false);
    data_ = static_cast<uint8_t*>(ptr);
    size_ = size;
    return data_;
}

void DataManager::Free(void* p) {
    SIMPLE_LOG_DEBUG("DataManager::Free is_owner: %s, input: %p, data: %p",
                     is_owner_ ? "True" : "False",
                     p,
                     data_);
    if (is_owner_ && p == data_) {
        fast_free(data_);
        size_ = 0U;
        data_ = nullptr;
    }
}

std::shared_ptr<DataManager> DataManager::Create() const {
    return std::static_pointer_cast<DataManager>(std::make_shared<DataManager>());
}

MStatus DataManager::SyncCache(bool io) {
    UNUSED_WARN(io);
    return MStatus::M_OK;
}

class DataBlock final {
public:
    DataBlock(const std::shared_ptr<DataManager>& data_ptr, bool in_use) : data_ptr_(data_ptr) {
        SetState(in_use);
    }

    std::shared_ptr<DataManager>& GetData() { return data_ptr_; }
    void SetState(const bool in_use) {
        in_use_     = in_use;
        time_stamp_ = TimeStamp();
    }
    bool IsUsing() const { return in_use_; }
    TimeStamp GetLastTimeUpdated() const { return time_stamp_; }

private:
    bool in_use_;
    TimeStamp time_stamp_;
    std::shared_ptr<DataManager> data_ptr_;
};

class MemoryPool final {
    static constexpr int MemTypeMax = 5;
    using DataBlockPtr              = std::shared_ptr<DataBlock>;
    using IdPool                    = std::unordered_map<uint32_t, DataBlockPtr, std::hash<int>>;
    using SizePool                  = std::unordered_map<uint32_t, IdPool, std::hash<int>>;
    using MemTypePool               = std::unordered_map<MemoryType, SizePool, std::hash<int>>;

public:
    static MemoryPool& GetInstance() {
        /// Prevent memory leaks here
        /// https://zhuanlan.zhihu.com/p/674795099
        static MemoryPool instance;
        return instance;
    }

public:
    ~MemoryPool() { pool_.clear(); }
    std::pair<uint32_t, std::shared_ptr<DataManager>> Allocate(const MemoryType mem_type,
                                                               const uint32_t size);
    void Release(const MemoryType mem_type, const uint32_t size, const uint32_t id);
    void UnusedTimeout(const int64_t timeout) { unused_timeout_ = timeout; }

    // Proactively returning memory to the system
    void Destory() { pool_.clear(); }

    // for debug
    void PrintPool();

private:
    MemoryPool() = default;
    void Collect(const MemoryType mem_type);
    bool Expand();
    bool Shrink();
    std::pair<uint32_t, std::shared_ptr<DataManager>> CreateDataMgr(const MemoryType mem_type,
                                                                    const uint32_t size);

    std::unordered_map<MemoryType, uint32_t, std::hash<int>> current_size_;
    MemTypePool pool_;
    uint32_t last_id_          = 0;
    uint32_t capacity_         = 5242880;
    uint32_t max_expand_times_ = 5;
    uint32_t expand_times_     = 0;
    int64_t unused_timeout_    = 5;
    std::mutex mutex_;
};

void MemoryPool::Collect(MemoryType mem_type) {
    auto type_it = pool_.find(mem_type);
    if (type_it == pool_.end()) {
        SIMPLE_LOG_DEBUG("collecting cache pool of non-exist memory type");
        return;
    }
    SIMPLE_LOG_DEBUG("   enter memory pool collect");
    auto& type_pool                         = type_it->second;
    std::vector<uint32_t> need_collect_size = {};
    for (auto& size_pool : type_pool) {
        std::vector<uint32_t> need_collect_id = {};
        for (auto& id_pool : size_pool.second) {
            if (id_pool.second->IsUsing()) {
                SIMPLE_LOG_WARN("   collect pool #BlockID_%i #BlockSize_%i is using",
                                id_pool.first,
                                size_pool.first);
                continue;
            }
            auto last_update          = id_pool.second->GetLastTimeUpdated();
            const auto now_time_stamp = TimeStamp();
            int64_t diff              = std::abs(last_update.tv_sec - now_time_stamp.tv_sec);
            SIMPLE_LOG_DEBUG("   time stamp diff: %i, limit: %i", diff, unused_timeout_);
            if (diff > unused_timeout_) {
                need_collect_id.push_back(id_pool.first);
            }
        }
        for (auto& id : need_collect_id) {
            SIMPLE_LOG_DEBUG("   collect #BlockId_%i", id);
            size_pool.second.erase(id);
            current_size_[mem_type] -= size_pool.first;
        }
        if (size_pool.second.empty()) {
            need_collect_size.push_back(size_pool.first);
        }
    }
    for (auto& size : need_collect_size) {
        SIMPLE_LOG_DEBUG("   collect #BlockSize_%i", size);
        type_pool.erase(size);
    }
}

bool MemoryPool::Expand() {
    if (expand_times_ < max_expand_times_) {
        SIMPLE_LOG_DEBUG("expanding data manager cache pool capacity");
        expand_times_++;
        capacity_ *= 2;
        return true;
    }
    return false;
}

bool MemoryPool::Shrink() {
    if (expand_times_ >= 1) {
        SIMPLE_LOG_DEBUG("shrinking data manager cache pool capacity");
        expand_times_--;
        capacity_ /= 2;
        return true;
    }
    return false;
}

std::pair<uint32_t, std::shared_ptr<DataManager>> MemoryPool::CreateDataMgr(MemoryType mem_type,
                                                                            uint32_t size) {

    auto it = current_size_.find(mem_type);
    if (it == current_size_.end()) {
        SIMPLE_LOG_DEBUG("#BlockType_%i is empty", static_cast<int>(mem_type));
        current_size_.insert(std::make_pair(mem_type, 0U));
    }
    while (current_size_[mem_type] + size > capacity_ && Expand()) {
        SIMPLE_LOG_DEBUG("#BlockType_%i is still over capacity", static_cast<int>(mem_type));
        if (!Expand()) {
            SIMPLE_LOG_ERROR("#BlockType_%i expand times over limits, %ivs%i",
                             static_cast<int>(mem_type),
                             expand_times_,
                             max_expand_times_);
            return std::make_pair(last_id_, nullptr);
        }
    }
    auto data_mgr = std::make_shared<DataManager>();
    if (!data_mgr) {
        SIMPLE_LOG_ERROR("MemoryPool::CreateDataMgr MemoryType: %i, size: %i failed",
                         static_cast<int>(mem_type),
                         size);
        return std::make_pair(last_id_, nullptr);
    }
    current_size_[mem_type] += size;
    data_mgr->Malloc(size);
    last_id_++;
    SIMPLE_LOG_INFO("Success Malloc #BlockID_%i, #BlockSize_%i", last_id_, size);
    return std::make_pair(last_id_, data_mgr);
}

std::pair<uint32_t, std::shared_ptr<DataManager>> MemoryPool::Allocate(const MemoryType mem_type,
                                                                       const uint32_t size) {
    SIMPLE_LOG_DEBUG(
        "MemoryPool::Allocate Start, MemoryType: %i, size: %i", static_cast<int>(mem_type), size);
    std::lock_guard<std::mutex> lock(mutex_);
    Collect(mem_type);
    while (current_size_[mem_type] < capacity_ / 2 && Shrink()) {
        SIMPLE_LOG_DEBUG("%i memory capacity shrink, %i", static_cast<int>(mem_type), capacity_);
    }

    auto type_it = pool_.find(mem_type);
    if (type_it == pool_.end()) {
        SIMPLE_LOG_DEBUG("cache pool for #BlockType_%i is empty", static_cast<int>(mem_type));
        auto ret        = CreateDataMgr(mem_type, size);
        auto data_block = std::make_shared<DataBlock>(ret.second, true);
        auto id_item    = std::make_pair(ret.first, data_block);
        IdPool id_pool;
        id_pool.insert(id_item);
        auto size_item = std::make_pair(size, id_pool);
        SizePool size_pool;
        size_pool.insert(size_item);
        auto mem_type_item = std::make_pair(mem_type, size_pool);
        pool_.insert(mem_type_item);
        return ret;
    }
    SIMPLE_LOG_DEBUG("MemoryPool::Allocate find #BlockType_%i", static_cast<int>(mem_type));

    auto& type_pool = type_it->second;
    auto size_it    = type_pool.find(size);
    if (size_it == type_pool.end()) {
        SIMPLE_LOG_DEBUG("MemoryType(%i) cache pool for #BlockSize_%i is empty",
                         static_cast<int>(mem_type),
                         size);
        auto ret        = CreateDataMgr(mem_type, size);
        auto data_block = std::make_shared<DataBlock>(ret.second, true);
        auto id_item    = std::make_pair(ret.first, data_block);
        IdPool id_pool;
        id_pool.insert(id_item);
        auto size_item = std::make_pair(size, id_pool);
        type_pool.insert(size_item);
        return ret;
    }
    SIMPLE_LOG_DEBUG("MemoryPool::Allocate find #BlockSize_%i", static_cast<int>(size));

    auto& size_pool = size_it->second;
    for (auto& item : size_pool) {
        if (!item.second->IsUsing()) {
            SIMPLE_LOG_DEBUG("reuse #BlockID_%i, #BlockSize_%i", item.first, size);
            item.second->SetState(true);
            return std::make_pair(item.first, item.second->GetData());
        }
    }

    if ([size_pool]() -> bool {
            for (auto& item : size_pool) {
                if (!item.second->IsUsing())
                    return false;
            }
            return true;
        }()) {
        auto ret        = CreateDataMgr(mem_type, size);
        auto data_block = std::make_shared<DataBlock>(ret.second, true);
        auto id_item    = std::make_pair(ret.first, data_block);
        size_pool.insert(id_item);
        SIMPLE_LOG_DEBUG("all #BlockSize_%i is using, so recreate #BlockID_%i", size, ret.first);
        return ret;
    }

    // this behavior is unexpected
    return std::make_pair(0U, nullptr);
}

void MemoryPool::Release(MemoryType mem_type, uint32_t size, uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto type_it = pool_.find(mem_type);
    if (type_it == pool_.end()) {
        SIMPLE_LOG_DEBUG("unable to find #BlockType_%i for this type", static_cast<int>(mem_type));
        return;
    }

    auto& size_pool = type_it->second;
    auto size_it    = size_pool.find(size);
    if (size_it == size_pool.end()) {
        SIMPLE_LOG_DEBUG("unable to find #BlockSize_%i for this size", size);
        return;
    }

    auto& id_pool = size_it->second;
    auto id_it    = id_pool.find(id);
    if (id_it == id_pool.end()) {
        SIMPLE_LOG_DEBUG("unable to find #BlockId_%i for this id", id);
        return;
    }

    id_it->second->SetState(false);
}

void MemoryPool::PrintPool() {
    std::stringstream ss;
    ss << "******************** MemoryPool Info ********************" << std::endl;
    ss << "capacity:     " << capacity_ << std::endl;
    ss << "expand_times: " << expand_times_ << std::endl;
    for (auto& type_pool : pool_) {
        uint32_t total = 0;
        ss << "#BlockType_" << static_cast<int>(type_pool.first) << ":" << std::endl;
        for (auto& size_pool : type_pool.second) {
            ss << "   #BlockSize_" << static_cast<int>(size_pool.first) << ": ";
            for (auto& id_pool : size_pool.second) {
                total += size_pool.first;
                ss << "      #BlockID_" << id_pool.first << ":"
                   << (id_pool.second->IsUsing() ? "(USING)" : "(UNUSE)") << "  ";
            }
            ss << std::endl;
        }
        ss << "#BlockType_" << static_cast<int>(type_pool.first) << "   total_size:   " << total
           << std::endl;
        ss << "#BlockType_" << static_cast<int>(type_pool.first)
           << "   current_size: " << current_size_[type_pool.first] << std::endl;
    }
    ss << "******************** MemoryPool End ********************" << std::endl;

    printf("%s", ss.str().c_str());
}

DataMgrCache::~DataMgrCache() {
    if (IsOwner()) {
        MemoryPool::GetInstance().Release(GetMemType(), size_, id_);
    }
}

void* DataMgrCache::Malloc(const uint32_t size) {
    SIMPLE_LOG_DEBUG("DataMgrCache::Malloc Start");

    auto ret      = MemoryPool::GetInstance().Allocate(GetMemType(), size);
    size_         = size;
    id_           = ret.first;
    data_manager_ = ret.second;

    // for debug
    // MemoryPool::GetInstance().PrintPool();

    SIMPLE_LOG_DEBUG("DataMgrCache::Malloc End");
    return data_manager_->GetDataPtr();
}

} // namespace base