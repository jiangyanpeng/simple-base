#ifndef SIMPLE_BASE_IMAGE_H_
#define SIMPLE_BASE_IMAGE_H_

#include "common.h"
#include "log.h"
#include "manager/data_manager.h"

#include <memory>
#include <new>
#include <string.h>
#include <vector>

namespace base {
class EXPORT_API Image final {
public:
    Image();

    ~Image() {}

    /// @brief Construct image without  memory allocate and data copy
    /// @param[in] width  : The width of image.
    /// @param[in] height : The height of image.
    /// @param[in] number : The width of image.
    /// @param[in] pixel_format : The format of image.
    /// @param[in] time_stamp : The time_stamp of image.
    /// @param[in] ptr : The address point of image
    /// @param[in] mem_type : The memory type of image.
    /// @note
    /// Only support number == 1 or batch size = 1
    /// Make sure the pointer and memory type correspond
    /// the channel of image determined by pixel_format, etc. bgr is 3
    Image(const uint32_t width,
          const uint32_t height,
          const uint32_t number,
          const PixelFormat pixel_format,
          const TimeStamp& time_stamp,
          const void* data,
          const MemoryType mem_type = MemoryType::MEM_ON_CPU,
          const bool use_cache      = false);

    /// @brief Construct image with memory allocate
    /// @param[in] width  : The width of image.
    /// @param[in] height : The height of image.
    /// @param[in] number : The width of image.
    /// @param[in] pixel_format : The format of image.
    /// @param[in] time_stamp : The time_stamp of image.
    /// @param[in] mem_type : The memory type of image.
    /// @param[in] use_cache : Whether use cache memory.
    /// @param[in] mem_alloced : Alloc memory insight.
    /// @note
    /// Only support number == 1 or batch size = 1
    /// the channel of image determined by pixel_format, etc. bgr is 3
    Image(const uint32_t width,
          const uint32_t height,
          const uint32_t number,
          const PixelFormat pixel_format,
          const MemoryType mem_type,
          const bool use_cache   = false,
          const bool mem_alloced = false);

    /// @brief Construct image with memory allocate
    /// @param[in] width  : The width of image.
    /// @param[in] height : The height of image.
    /// @param[in] number : The width of image.
    /// @param[in] pixel_format : The format of image.
    /// @param[in] time_stamp : The time_stamp of image.
    /// @param[in] mem_type : The memory type of image.
    /// @note
    /// Only support number == 1 or batch size = 1
    /// the channel of image determined by pixel_format, etc. bgr is 3
    Image(const uint32_t width,
          const uint32_t height,
          const uint32_t number,
          const PixelFormat pixel_format,
          const TimeStamp& time_stamp,
          const MemoryType mem_type = MemoryType::MEM_ON_CPU,
          const bool use_cache      = false);

    /// @brief Construct  with specified format, only memory alloc, no data copy
    /// @param[in] pixel_format : The format of image.
    Image(const Image& other, const PixelFormat format, const bool use_cache = false)
        : Image(other.GetWidth(),
                other.GetHeight(),
                other.GetNumber(),
                format,
                other.GetTimestamp(),
                other.data_manager_->GetMemType(),
                use_cache) {}

    /// @brief Construct  with specified  h w, only memory alloc, no data copy
    /// @param[in] width  : The width of image.
    /// @param[in] height : The height of image.
    Image(const Image& other,
          const uint32_t width,
          const uint32_t height,
          const bool use_cache = false)
        : Image(width,
                height,
                other.GetNumber(),
                other.GetPixelFormat(),
                other.GetTimestamp(),
                other.data_manager_->GetMemType(),
                use_cache) {}

    /// @brief Construct  with specified  mem type , only memory alloc, no data copy
    /// @param[in] mem_type  : mem_type.
    Image(const Image& other, const MemoryType mem_type, const bool use_cache = false)
        : Image(other.GetWidth(),
                other.GetHeight(),
                other.GetNumber(),
                other.GetPixelFormat(),
                other.GetTimestamp(),
                mem_type,
                use_cache) {}

    /// @brief Split image with index, No data copy
    /// @param[in] idx : The index of number.
    /// @param[out] image_out : output Image.
    /// @note
    /// Only support number == 1 or batch size = 1
    /// image_out will not take control of memory.
    /// the memory will be controled by raw image.
    /// User need to make sure split_image is free before raw_image
    MStatus ImageSplit(const uint32_t idx, Image& image_out) const;


    /// @brief Split image with channel, No data copy
    /// @param[in] idx : The index of number.
    /// @param[in] channel : The channel of number.
    /// @param[out] image_out : output Image.
    /// @note
    /// image_out will not take control of memory.
    /// the memory will be controled by raw image.
    /// User need to make sure split_image is free before raw_image
    MStatus ImageSplitChannel(const uint32_t batch, const uint32_t channel, Image& image_out) const;

    /// @brief Reshape image without changing data
    /// @param[in] width  : The width of image.
    /// @param[in] height : The height of image.
    /// @param[in] number : The width of image.
    /// @param[in] pixel_format : The format of image.
    /// @note
    /// etc .use for YUV reshape to Y
    MStatus ImageReshape(const uint32_t width,
                         const uint32_t height,
                         const uint32_t number,
                         const PixelFormat pixel_format);

    /// @brief repalce data manager
    MStatus ImageDataManagerReplace(const std::shared_ptr<DataManager>& data_mgr);

    bool operator==(const Image& other);
    bool operator!=(const Image& other);

    /// @brief Get Address pointer of image
    /// @param[in] n  : The idx of image number
    template <typename T>
    inline T* GetData(size_t n = 0) const {
        if (number_ <= 0U || data_manager_ == nullptr) {
            return nullptr;
        }
        uint8_t* data_u8 = static_cast<uint8_t*>(data_manager_->GetDataPtr());
        void* data       = static_cast<void*>(data_u8 + n * nscalar_);
        return static_cast<T*>(data);
    }

    /// @brief Get width of image
    inline uint32_t GetWidth() const { return width_; }

    /// @brief Get height of image
    inline uint32_t GetHeight() const { return height_; }

    /// @brief Get channel of image
    inline uint32_t GetChannel() const { return channel_; }

    /// @brief Get stride of image or image width * channel
    inline uint32_t GetStride() const { return stride_; }

    /// @brief Get numbers of image
    inline uint32_t GetNumber() const { return number_; }

    /// @brief Get typesize of image
    /// @note
    /// etc. uchar = 1, float = 4
    inline uint32_t GetTypeSize() const { return type_size_; }

    /// @brief Get typesize of image
    /// @note
    /// etc. sizeof(float) * width * hight * channal
    inline uint32_t GetScalar() const { return nscalar_; }

    /// @brief Get size of image
    /// @note
    /// Only support number == 1 or batch size = 1
    /// etc. uchar of image as width * height * channel * number
    inline uint32_t GetSize() const { return number_ * nscalar_; }

    /// @brief Get data manager of image
    inline const std::shared_ptr<DataManager>& GetDataManager() const { return data_manager_; }

    /// @brief Get memory type of image
    /// @note
    /// etc. uchar of image as CPU / CUDA
    inline const MemoryType& GetMemType() const { return data_manager_->GetMemType(); }
    inline const std::string& GetMemTypeStr() const { return data_manager_->GetMemTypeStr(); }

    /// @brief Get pixel format of image
    inline const PixelFormat& GetPixelFormat() const { return pixel_format_; }
    const std::string& GetPixelFormatStr() const { return pixel_format_str_; }

    /// @brief Get time stamp of image
    inline const TimeStamp GetTimestamp() const { return time_stamp_; }

    /// @brief Deep Clone of Image
    Image Clone() const { return Clone(data_manager_->GetMemType()); }
    Image Clone(MemoryType type) const;

private:
    MStatus InitImageParamters();
    MStatus CreatDataManager(const MemoryType mem_type);

    uint32_t width_;
    uint32_t height_;
    uint32_t stride_;
    uint32_t nscalar_;
    uint32_t number_;
    uint32_t channel_;
    uint32_t type_size_;
    PixelFormat pixel_format_;
    std::string pixel_format_str_;
    TimeStamp time_stamp_;
    std::shared_ptr<DataManager> data_manager_;
    
    bool use_cache_{false};
    bool init_done_{false};
};

inline std::string LogImage(std::string prefix, const Image& image) {
    char ret[1024];
    sprintf(ret,
            "{%s} Image: Number {%u}, Width {%u}, Height {%u}, Stride {%u}, Scalar {%u}, "
            "MemType {%s}, Format {%s}, Data 0x{%lu}",
            prefix.c_str(),
            image.GetNumber(),
            image.GetWidth(),
            image.GetHeight(),
            image.GetStride(),
            image.GetScalar(),
            image.GetMemTypeStr().c_str(),
            image.GetPixelFormatStr().c_str(),
            reinterpret_cast<uint64_t>(image.GetData<uint8_t>(0)));
    return std::string(ret);
}

} // namespace base
#endif // SIMPLE_BASE_IMAGE_H_
