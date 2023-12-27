#ifndef SIMPLE_BASE_DATA_MANAGER_H_
#define SIMPLE_BASE_DATA_MANAGER_H_

#include "common.h"
#include "log.h"

#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <uchar.h>
#include <unordered_map>

namespace base {

#define MEMTYPE_CPU ("CPU")
#define MEMTYPE_OCL ("OCL")
#define MEMTYPE_HEXAGON_DSP ("HEXAGON_DSP")
#define MEMTYPE_CUDA_HOST ("CUDA_HOST")
#define MEMTYPE_CUDA_DEV ("CUDA_DEV")
#define MEMTYPE_INVALID ("INVALID")

// clang-format off
const static std::unordered_map<std::string, MemoryType> mem_type_map_ = {
    {MEMTYPE_CPU,         MemoryType::M_MEM_ON_CPU        },
    {MEMTYPE_CUDA_HOST,   MemoryType::M_MEM_ON_CUDA_HOST  },
    {MEMTYPE_CUDA_DEV,    MemoryType::M_MEM_ON_CUDA_DEV   },
    {MEMTYPE_OCL,         MemoryType::M_MEM_ON_OCL        },
    {MEMTYPE_HEXAGON_DSP, MemoryType::M_MEM_ON_HEXAGON_DSP}
};
// clang-format on

/* align up to z^n */
template <typename T_>
inline T_ AlignUp2N(const T_ v, uint32_t r) {
    return (((v + ((static_cast<T_>(0x1) << r) - static_cast<T_>(0x1))) >> r) << r);
}
/* align down to z^n */
template <typename T_>
inline T_ AlignDownN(const T_ v, uint32_t r) {
    return ((v >> r) << r);
}

#define ALIGN_UP_16(x) AlignUp2N(x, 4)
#define ALIGN_DOWN_16(x) AlignDownN(x, 4)

#define ALIGN_UP_2(x) AlignUp2N(x, 1)
#define ALIGN_DOWN_2(x) AlignDownN(x, 1)

#define IS_ALIGN_16(x) (((x) & 0xF) == 0x0)
#define IS_ALIGN_2(x) (((x) & 0x1) == 0x0)

// the alignment of all the allocated buffers
#if AVX512
#define MALLOC_ALIGN 64
#elif AVX
#define MALLOC_ALIGN 32
#else
#define MALLOC_ALIGN 16
#endif

// we have some optimized kernels that may overread buffer a bit in loop
// it is common to interleave next-loop data load with arithmetic instructions
// allocating more bytes keeps us safe from SEGV_ACCERR failure
#define MALLOC_OVERREAD 64

typedef uint8_t* u_char_ptr;
// Aligns a pointer to the specified number of bytes
// ptr Aligned pointer
// n Alignment size that must be a power of two
static inline u_char_ptr* align_ptr(u_char_ptr* ptr, int n /*(int)sizeof(u_char_ptr)*/) {
    return (u_char_ptr*)(((size_t)ptr + n - 1) & -n);
}

// Aligns a buffer size to the specified number of bytes
// The function returns the minimum number that is greater or equal to sz and is divisible by n
// sz Buffer size to align
// n Alignment size that must be a power of two
static inline size_t align_size(size_t sz, int n) {
    return (sz + n - 1) & -n;
}

static inline void* fast_malloc(size_t size) {
    uint8_t* udata = (uint8_t*)malloc(size + sizeof(void*) + MALLOC_ALIGN + MALLOC_OVERREAD);
    if (!udata) {
        return 0;
    }
    uint8_t** adata = align_ptr((uint8_t**)udata + 1, MALLOC_ALIGN);
    adata[-1]       = udata;
    return adata;
}

static inline void fast_free(void* ptr) {
    if (ptr) {
        uint8_t* udata = ((uint8_t**)ptr)[-1];
        free(udata);
    }
}

class EXPORT_API DataManager {
public:
    DataManager()
        : mem_type_(MemoryType::M_MEM_ON_CPU),
          mem_type_str_(MEMTYPE_CPU),
          is_owner_(false),
          data_{nullptr},
          size_(0U) {}

    virtual ~DataManager() {}

    virtual void* Malloc(const uint32_t size);
    virtual std::shared_ptr<DataManager> Create() const;
    virtual MStatus SyncCache(bool io = true);
    virtual uint32_t GetSize() const { return size_; }
    virtual void* GetDataPtr() const { return reinterpret_cast<void*>(data_); }

    void* Setptr(void* ptr, uint32_t size);
    void Free();
    inline const MemoryType& GetMemType() const { return mem_type_; }
    inline const std::string& GetMemTypeStr() const { return mem_type_str_; }

    inline void SetMemType(const std::string& type) {
        mem_type_str_ = type;
        mem_type_     = MemTypeStrToMemType(mem_type_str_);
    }

    inline bool IsOwner() const { return is_owner_; };
    inline void SetOwer(bool owner) { is_owner_ = owner; }
    static const std::string MemTypeToMemTypeStr(const MemoryType type) {
        for (auto& item : mem_type_map_) {
            if (item.second == type) {
                return item.first;
            }
        }
        return "invalid";
    }

    static MemoryType MemTypeStrToMemType(const std::string& type) {
        auto it = mem_type_map_.find(type);
        if (it == mem_type_map_.end()) {
            SIMPLE_LOG_ERROR("unsupport mem type %s\n", type.c_str());
            return MemoryType::M_MEM_ON_MEMORY_MAX;
        }
        return it->second;
    }

private:
    MemoryType mem_type_;
    std::string mem_type_str_;
    bool is_owner_;

    uint8_t* data_;
    uint32_t size_;
};


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

class DataMgrCache final : public DataManager {
public:
    DataMgrCache(std::string mem_type) : DataManager() { SetMemType(mem_type); }
    ~DataMgrCache();
    void* Malloc(const uint32_t size) override;

    std::shared_ptr<DataManager> Create() const override { return data_manager_->Create(); }
    void* GetDataPtr() const override { return data_manager_->GetDataPtr(); }
    MStatus SyncCache(bool io = true) override { return data_manager_->SyncCache(io); };
    uint32_t GetSize() const override { return data_manager_->GetSize(); };

private:
    uint32_t size_                             = 0;
    uint32_t id_                               = 0;
    std::shared_ptr<DataManager> data_manager_ = nullptr;
};


} // namespace base
#endif // SIMPLE_BASE_DATA_MANAGER_H_