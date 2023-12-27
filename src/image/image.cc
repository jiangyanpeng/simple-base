#ifdef _MSC_VER
#include <algorithm>
#endif
#include "image/image.h"
#include "log.h"
#include "manager/data_manager.h"

#include <fstream>
#include <iomanip>

namespace base {

Image::Image()
    : number_{0},
      width_{0},
      height_{0},
      channel_{0},
      stride_{0},
      nscalar_{0},
      type_size_{0},
      time_stamp_{},
      pixel_format_{},
      pixel_format_str_{},
      data_manager_{nullptr},
      use_cache_{false},
      init_done_{false} {}

Image::Image(const uint32_t width,
             const uint32_t height,
             const uint32_t number,
             const PixelFormat format,
             const TimeStamp& time_stamp,
             const void* data,
             const MemoryType mem_type,
             const bool use_cache) {
    width_        = width;
    height_       = height;
    number_       = number;
    pixel_format_ = format;
    time_stamp_   = time_stamp;
    use_cache_    = use_cache;

    if (nullptr == data) {
        SIMPLE_LOG_ERROR("construct image failed, input ptr nullptr");
        return;
    }

    if (this->CreatDataManager(mem_type) != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image manager failed");
        return;
    }

    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image paramters failed");
        return;
    }
    this->data_manager_->Setptr(const_cast<void*>(data), this->nscalar_ * this->number_);
    init_done_ = true;
}
Image::Image(const uint32_t width,
             const uint32_t height,
             const uint32_t number,
             const PixelFormat pixel_format,
             const MemoryType mem_type,
             const bool use_cache,
             const bool mem_alloced)
    : number_{number},
      width_{width},
      height_{height},
      pixel_format_{pixel_format},
      use_cache_{use_cache} {

    if (this->CreatDataManager(mem_type) != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image manager failed");
        return;
    }

    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image paramters failed");
        return;
    }

    if (!mem_alloced) {
        this->data_manager_->Malloc(this->nscalar_ * this->number_);
    }
    init_done_ = true;
}

Image::Image(const uint32_t width,
             const uint32_t height,
             const uint32_t number,
             const PixelFormat format,
             const TimeStamp& time_stamp,
             const MemoryType mem_type,
             const bool use_cache) {
    width_        = width;
    height_       = height;
    number_       = number;
    pixel_format_ = format;
    time_stamp_   = time_stamp;
    use_cache_    = use_cache;

    if (this->CreatDataManager(mem_type) != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image manager failed");
        return;
    }

    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("construct image failed, init image paramters failed");
        return;
    }

    this->data_manager_->Malloc(this->nscalar_ * this->number_);
    init_done_ = true;
}

MStatus Image::ImageSplit(const uint32_t idx, Image& image_out) const {
    SIMPLE_LOG_WARN("now can't support ImageSplit for %i", idx);
    UNUSED_WARN(idx);
    UNUSED_WARN(image_out);
    return MStatus::M_OK;
}

MStatus
Image::ImageSplitChannel(const uint32_t batch, const uint32_t channel, Image& image_out) const {
    if (!init_done_) {
        SIMPLE_LOG_ERROR("splict channel failed, construct image no init success");
        return MStatus::M_INTERNAL_FAILED;
    }
    if (channel >= this->channel_) {
        SIMPLE_LOG_ERROR(
            "input channel %i is more than image's channel %i", channel, this->channel_);
        return MStatus::M_INVALID_ARG;
    }

    if (!(this->pixel_format_ == PixelFormat::M_PIX_FMT_BGR888_PLANAR ||
          this->pixel_format_ == PixelFormat::M_PIX_FMT_BGR161616_PLANAR ||
          this->pixel_format_ == PixelFormat::M_PIX_FMT_BGR323232_PLANAR ||
          this->pixel_format_ == PixelFormat::M_PIX_FMT_RGB888_PLANAR ||
          this->pixel_format_ == PixelFormat::M_PIX_FMT_RGB161616_PLANAR ||
          this->pixel_format_ == PixelFormat::M_PIX_FMT_RGB323232_PLANAR)) {
        SIMPLE_LOG_ERROR("ImageSplitChannel cant't support format %s",
                         this->pixel_format_str_.c_str());
        return MStatus::M_INVALID_ARG;
    }

    auto mem_type = this->data_manager_->GetMemType();
    auto offset   = (mem_type == MemoryType::M_MEM_ON_OCL)
                        ? 0
                        : batch * this->nscalar_ + channel * (this->width_ * this->height_);

    Image split_image(this->width_,
                      this->height_,
                      1,
                      PixelFormat::M_PIX_FMT_GRAY8,
                      this->time_stamp_,
                      static_cast<uint8_t*>(this->data_manager_->GetDataPtr()) + offset,
                      mem_type);

    // todo use move operator to optimizer
    image_out = split_image;
    return MStatus::M_OK;
}

bool Image::operator==(const Image& other) {
    if (this->pixel_format_ != other.pixel_format_ || this->height_ != other.height_ ||
        this->width_ != other.width_ || this->number_ != other.number_ ||
        this->stride_ != other.stride_ || this->nscalar_ != other.nscalar_ ||
        this->channel_ != other.channel_ || this->type_size_ != other.type_size_ ||
        this->GetMemType() != other.GetMemType() ||
        this->GetData<uint8_t>() != other.GetData<uint8_t>()) {
        return false;
    }
    return true;
}
bool Image::operator!=(const Image& other) {
    return (*this == other) ? false : true;
}

MStatus Image::InitImageParamters() {
    pixel_format_str_ = FormatStr[static_cast<int>(pixel_format_) - 1];

    auto SetParams = [&](uint32_t c, uint32_t t, bool chanel_on_stide) -> bool {
        channel_   = c;
        type_size_ = t;
        stride_    = width_ * type_size_;
        stride_    = (chanel_on_stide) ? stride_ * channel_ : stride_;
        nscalar_   = height_ * stride_;
        nscalar_   = (chanel_on_stide) ? nscalar_ : nscalar_ * channel_;
        return true;
    };

    MStatus m_status = MStatus::M_OK;
    switch (pixel_format_) {
        case PixelFormat::M_PIX_FMT_GRAY8: {
            SetParams(1, 1, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_BGR888:
        case PixelFormat::M_PIX_FMT_RGB888: {
            SetParams(3, 1, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_RGB888_PLANAR:
        case PixelFormat::M_PIX_FMT_BGR888_PLANAR: {
            SetParams(3, 1, false);
            break;
        }
        case PixelFormat::M_PIX_FMT_YUV420P:
        case PixelFormat::M_PIX_FMT_NV12:
        case PixelFormat::M_PIX_FMT_NV21:
        case PixelFormat::M_PIX_FMT_YV12:
        case PixelFormat::M_PIX_FMT_YU12: {
            SetParams(1, 1, true);
            nscalar_ = nscalar_ * 3 / 2;
            break;
        }
        case PixelFormat::M_PIX_FMT_NV12_DETACH:
        case PixelFormat::M_PIX_FMT_NV21_DETACH: {
            SetParams(1, 1, true);
            nscalar_ = nscalar_ * 3 / 2;
            break;
        }
        case PixelFormat::M_PIX_FMT_BGRA8888:
        case PixelFormat::M_PIX_FMT_RGBA8888: {
            SetParams(4, 1, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_GRAY32: {
            SetParams(1, 4, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_BGR323232:
        case PixelFormat::M_PIX_FMT_RGB323232: {
            SetParams(3, 4, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_RGB323232_PLANAR:
        case PixelFormat::M_PIX_FMT_BGR323232_PLANAR: {
            SetParams(3, 4, false);
            break;
        }
        case PixelFormat::M_PIX_FMT_GRAY16: {
            SetParams(1, 2, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_BGR161616:
        case PixelFormat::M_PIX_FMT_RGB161616: {
            SetParams(3, 2, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_RGB161616_PLANAR:
        case PixelFormat::M_PIX_FMT_BGR161616_PLANAR: {
            SetParams(3, 2, false);
            break;
        }
        case PixelFormat::M_PIX_FMT_YUYV:
        case PixelFormat::M_PIX_FMT_UYVY: {
            SetParams(2, 1, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_FLOAT32C4: {
            SetParams(4, 4, true);
            break;
        }
        case PixelFormat::M_PIX_FMT_MAX:
        default: {
            SIMPLE_LOG_ERROR("format %s illegal", pixel_format_str_.c_str());
            SetParams(3, 1, true);
            m_status = MStatus::M_INVALID_FILE_FORMAT;
        }
    }

    SIMPLE_LOG_DEBUG("%s", LogImage("InitImageParamters", *this));
    return m_status;
}

Image Image::Clone(MemoryType type) const {
    this->GetDataManager()->SyncCache(false);
    Image target(width_, height_, number_, pixel_format_, time_stamp_, type);
    // const auto src_data_height = static_cast<uint32_t>(this->nscalar_) / this->stride_;
    // const auto dst_data_height = static_cast<uint32_t>(target.nscalar_) / target.stride_;
    // const auto batch           = this->number_;

    // auto src_last_batch_height = src_data_height - (batch * this->aligned_height_);
    // auto dst_last_batch_height = dst_data_height - (batch * target.aligned_height_);
    // if ((src_last_batch_height > 0) != (dst_last_batch_height > 0)) {
    //     SDK_LOG_ERROR("Image CopyDataInner error last batch: {} vs {}",
    //                   src_last_batch_height,
    //                   dst_last_batch_height);
    //     return target;
    // }
    // const auto last_batch_height = (std::min)(src_last_batch_height, dst_last_batch_height);
    // const auto min_stride        = (std::min)(this->stride_, target.stride_);
    // const auto pad_num           = target.stride_ - min_stride; // zero if target stride is min

    // for (size_t i = 0; i < number_; ++i) {
    //     auto src = this->GetData<uint8_t>(i);
    //     auto dst = target.GetData<uint8_t>(i);

    //     // optimize whole image copy
    //     if (src_data_height == dst_data_height && this->stride_ == target.stride_) {
    //         memcpy(dst, src, this->nscalar_);
    //         continue;
    //     }

    //     for (uint32_t b = 0; b < batch; b++) {
    //         auto src_row = src;
    //         auto dst_row = dst;
    //         for (uint32_t h = 0; h < this->height_; h++) {
    //             memcpy(dst_row, src_row, min_stride);
    //             // set to zero to avoid resize result rand, zero is safe for pad_num
    //             memset(dst_row + min_stride, 0, pad_num);
    //             src_row += this->stride_;
    //             dst_row += target.stride_;
    //         }
    //         src += this->aligned_height_ * this->stride_;
    //         dst += target.aligned_height_ * target.stride_;
    //     }

    //     if (last_batch_height > 0) {
    //         for (uint32_t h = 0; h < last_batch_height; h++) {
    //             memcpy(dst, src, min_stride);
    //             src += this->stride_;
    //             dst += target.stride_;
    //         }
    //     }
    // }
    // target.GetDataManager()->SyncCache(true);
    return target;
}

MStatus Image::ImageReshape(const uint32_t width,
                            const uint32_t height,
                            const uint32_t number,
                            const PixelFormat pixel_format) {
    if (!init_done_) {
        SIMPLE_LOG_ERROR("splict channel failed, construct image no init success");
        return MStatus::M_INTERNAL_FAILED;
    }
    this->pixel_format_ = pixel_format;
    this->width_        = width;
    this->height_       = height;
    this->number_       = number;

    if (this->InitImageParamters() != MStatus::M_OK) {
        SIMPLE_LOG_ERROR("ImageReshape failed, init image paramters failed");
        return MStatus::M_FAILED;
    }

    // todo image shape change
    return MStatus::M_OK;
}

MStatus Image::CreatDataManager(const MemoryType mem_type) {
    std::string mem_type_str = DataManager::MemTypeToMemTypeStr(mem_type);
    SIMPLE_LOG_DEBUG("Image::CreatDataManager %s", mem_type_str.c_str());

    if (nullptr == this->data_manager_) {
        if (use_cache_) {
            SIMPLE_LOG_ERROR("now not support cache manager %s memory", mem_type_str.c_str());
            return MStatus::M_NOT_SUPPORT;
        } else {
            this->data_manager_ = std::make_shared<DataManager>();
        }
    }
    if (nullptr == this->data_manager_) {
        SIMPLE_LOG_ERROR("%s data_manager is nullptr", mem_type_str.c_str());
        return M_INTERNAL_FAILED;
    }
    return MStatus::M_OK;
}

MStatus Image::ImageDataManagerReplace(const std::shared_ptr<DataManager>& data_mgr) {
    if (!init_done_) {
        SIMPLE_LOG_ERROR("splict channel failed, construct image no init success");
        return MStatus::M_INTERNAL_FAILED;
    }
    if (nullptr == this->GetDataManager() || nullptr == data_mgr) {
        SIMPLE_LOG_ERROR("ImageDataManagerReplace failed, data_mgr is nullptr");
        return MStatus::M_FAILED;
    }
    if (this->GetDataManager()->GetSize() != data_mgr->GetSize()) {
        SIMPLE_LOG_ERROR("ImageDataManagerReplace failed, data_mgr size err %ivs%i",
                         this->GetDataManager()->GetSize(),
                         data_mgr->GetSize());
        return MStatus::M_FAILED;
    }
    data_manager_ = std::move(data_mgr);
    return MStatus::M_OK;
}

} // namespace base
