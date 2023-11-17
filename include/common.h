#ifndef SIMPLE_BASE_COMMON_H_
#define SIMPLE_BASE_COMMON_H_

#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) > (b)) ? (b) : (a))
#define swap(a, b) a ^= b, b ^ = a, a ^= b;

#define SATURATE_CAST_UCHAR(X) (uint8_t) min(max((int)(X), 0), 255);

#ifdef SIMPLE_EXPORTS
#ifdef _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif
#else
#define EXPORT_API
#endif

#if defined(__linux__) || defined(__android__) || defined(__QNX__)
#define PROGRAM_STRING(N) _Pragma(#N)
#define IGNORED(arg) PROGRAM_STRING(GCC diagnostic ignored arg)
#define PUSH_IGNORE(...) _Pragma("GCC diagnostic push") __VA_ARGS__
#define POP_IGNORE _Pragma("GCC diagnostic pop")
#else
#define PROGRAM_STRING(N)
#define IGNORED(arg)
#define PUSH_IGNORE(...)
#define POP_IGNORE
#endif

#if defined(__GNUC__) || defined(__ICL) || defined(__clang__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#if defined(_MSC_VER) || defined(__CODEGEARC__)
#define SIMPLE_INLINE __forceinline
#elif defined(__GNUC__)
#define SIMPLE_INLINE inline __attribute__((always_inline))
#else
#error This platform is unsupported!
#endif

/** A struct to show time  */
typedef struct TimeStamp {
    long int tv_sec;  /**< second */
    long int tv_usec; /**< microsecond */
} TimeStamp;

/** A enum of memory type */
typedef enum MemoryType {
    M_MEM_ON_CPU         = 0, /**< memory type normal cpu */
    M_MEM_ON_OCL         = 1, /**< memory type opencl */
    M_MEM_ON_HEXAGON_DSP = 2, /**< memory type hexagon dsp */
    M_MEM_ON_CUDA_HOST   = 3, /**< memory type normal cuda */
    M_MEM_ON_CUDA_DEV    = 4, /**< memory type normal cuda */
    M_MEM_ON_MEMORY_MAX  = 5  /**< the max number of memory type */
} MemoryType;

/** A enum to represent pixel format */
typedef enum PixelFormat {
    M_PIX_FMT_GRAY8            = 0,  /**< gray8  */
    M_PIX_FMT_RGBA8888         = 1,  /**< rgba8888 */
    M_PIX_FMT_RGB888           = 2,  /**< rgb888 */
    M_PIX_FMT_RGB888_PLANAR    = 3,  /**< rgb888 RRRRRR:GGGGGG:BBBBBB */
    M_PIX_FMT_BGRA8888         = 4,  /**< bgra8888 */
    M_PIX_FMT_BGR888           = 5,  /**< bgr888 */
    M_PIX_FMT_BGR888_PLANAR    = 6,  /**< bgr888 BBBBBB:GGGGGG:RRRRRR */
    M_PIX_FMT_YUV420P          = 7,  /**< yuv420p */
    M_PIX_FMT_NV12             = 8,  /**< YUV  4:2:0 YYYY:UV */
    M_PIX_FMT_NV21             = 9,  /**< YUV  4:2:0 YYYY:VU */
    M_PIX_FMT_GRAY32           = 10, /**< gray32*/
    M_PIX_FMT_RGB323232        = 11, /**< rgb323232 fp32*/
    M_PIX_FMT_RGB323232_PLANAR = 12, /**< rgb323232 fp32  RRRRRR:GGGGGG:BBBBBB*/
    M_PIX_FMT_BGR323232        = 13, /**< bgr323232 fp32*/
    M_PIX_FMT_BGR323232_PLANAR = 14, /**< bgr323232 fp32 BBBBBB:GGGGGG:RRRRRR*/
    M_PIX_FMT_GRAY16           = 15, /**< gray16*/
    M_PIX_FMT_RGB161616        = 16, /**< rgb161616 fp16*/
    M_PIX_FMT_RGB161616_PLANAR = 17, /**< rgb161616 fp16  RRRRRR:GGGGGG:BBBBBB*/
    M_PIX_FMT_BGR161616        = 18, /**< bgr161616 fp16*/
    M_PIX_FMT_BGR161616_PLANAR = 19, /**< bgr161616 fp16 BBBBBB:GGGGGG:RRRRRR*/
    M_PIX_FMT_FLOAT32C4        = 20, /**< fp32 channel ==4 */
    M_PIX_FMT_NV12_DETACH      = 21, /**< Y/UV not Contiguous memory*/
    M_PIX_FMT_NV21_DETACH      = 22, /**< Y/UV not Contiguous memory*/
    M_PIX_FMT_YUYV             = 23, /**< YUV422 PACKED*/
    M_PIX_FMT_UYVY             = 24, /**< YUV422 PACKED*/
    M_PIX_FMT_YV12             = 25, /**< YUV  4:2:0 YYYYYYYY:VVUU */
    M_PIX_FMT_YU12             = 26, /**< YUV  4:2:0 YYYYYYYY:UUVV */
    M_PIX_FMT_MAX              = 27  /**< pixel format is invalid */
} PixelFormat;

typedef enum MStatus {
    M_OK                  = 0,
    M_FAILED              = 1,
    M_INVALID_ARG         = 2,
    M_FILE_NOT_FOUND      = 3,
    M_INVALID_FILE_FORMAT = 4,
    M_INTERNAL_FAILED     = 5,
    M_OUT_OF_MEMORY       = 6,
    M_NOT_SUPPORT         = 7,
    M_MAX_STATUS          = 8
} MStatus;

/** enumerator to indicate primitive numeric types */
typedef enum DataType {
    M_DATA_TYPE_BYTE    = 0,  ///< obscure bytes (e.g. encoded JPEG)
    M_DATA_TYPE_BOOL    = 1,  ///< boolean (1-byte)
    M_DATA_TYPE_INT8    = 2,  ///< 8-bit signed integer
    M_DATA_TYPE_INT16   = 3,  ///< 16-bit signed integer
    M_DATA_TYPE_INT32   = 4,  ///< 32-bit signed integer
    M_DATA_TYPE_INT64   = 5,  ///< 64-bit signed integer
    M_DATA_TYPE_UINT8   = 6,  ///< 8-bit unsigned integer
    M_DATA_TYPE_UINT16  = 7,  ///< 16-bit unsigned integer
    M_DATA_TYPE_UINT32  = 8,  ///< 32-bit unsigned integer
    M_DATA_TYPE_UINT64  = 9,  ///< 64-bit unsigned integer
    M_DATA_TYPE_FLOAT16 = 10, ///< 16-bit floating point real number
    M_DATA_TYPE_FLOAT32 = 11, ///< 16-bit floating point real number
    M_DATA_TYPE_FLOAT64 = 12, ///< 64-bit floating point real number
    M_DATA_TYPE_FIX16   = 13, ///< 16-bit floating point real number
    M_DATA_TYPE_FIX32   = 14, ///< 16-bit floating point real number
    M_DATA_TYPE_MAX     = 15, ///< 64-bit floating point real number
} DataType;

typedef enum TensorLayout {
    M_LAYOUT_NCHW = 0,
    M_LAYOUT_NHWC = 1,
    M_LAYOUT_MAX  = 2,
} TensorLayout;
#ifdef __cplusplus
}
#endif // __cplusplus

// clang-format off
const static std::string MemTypeStr[M_MEM_ON_MEMORY_MAX] = {
    "CPU", "OCL", "HEXAGON_DSP", "CUDA_HOST", "CUDA_DEV",
};
const static std::string FormatStr[M_PIX_FMT_MAX] = {
    "GRAY8",            "RGBA8888",         "RGB888",           
    "RGB888_PLANAR",    "BGRA8888",         "BGR888",    
    "BGR888_PLANAR",    "YUV420P",          "NV12",          
    "NV21",             "GRAY32",           "RGB323232",
    "RGB323232_PLANAR", "BGR323232",        "BGR323232_PLANAR",
    "GRAY16",           "RGB161616",        "RGB161616_PLANAR", 
    "BGR161616",        "BGR161616_PLANAR", "FLOAT32C4",
    "NV12_DETACH",      "NV21_DETACH",      "YUYV",          
    "UYVY",             "YV12",             "YU12"};
const static std::string MStatusStr[M_MAX_STATUS] = {
    "OK",             "FAILED",              "INVALID_ARG",   
    "FILE_NOT_FOUND", "INVALID_FILE_FORMAT", "INTERNAL_FAILED", 
    "OUT_OF_MEMORY",  "NOT_SUPPORT"};
const static std::string DataTypeStr[M_DATA_TYPE_MAX] = {
    "BYTE",    "BOOL",    "INT8",    "INT16",  "INT32",
    "INT64",   "UINT8",   "UINT16",  "UINT32", "UINT64",
    "FLOAT16", "FLOAT32", "FLOAT64", "FIX16",  "FIX32"};
const static std::string TensorLayoutStr[M_LAYOUT_MAX] = {
    "NCHW",    "NHWC"};
#endif // SIMPLE_BASE_COMMON_H_