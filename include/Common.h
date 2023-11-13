#ifndef SIMPLE_BASE_COMMON_H_
#define SIMPLE_BASE_COMMON_H_

#include <stdint.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SIMPLE_EXPORTS
#ifdef _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif
#else
#define EXPORT_API
#endif

#if defined(__linux__) ||  defined(__android__) || defined(__QNX__)
#define PROGRAM_STRING(N) _Pragma(#N)
#define IGNORED(arg) PROGRAM_STRING(GCC diagnostic ignored arg)
#define PUSH_IGNORE(...) _Pragma("GCC diagnostic push")  __VA_ARGS__
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

/** A struct to show time  */
typedef struct TimeStamp {
    long int tv_sec;  /**< second */
    long int tv_usec; /**< microsecond */
} TimeStamp;

/** A enum of memory type */
typedef enum MemoryType {
    MEM_ON_CPU = 0,                 /**< memory type normal cpu */
    MEM_ON_OCL = 1,                 /**< memory type opencl */
    MEM_ON_HEXAGON_DSP = 2,         /**< memory type hexagon dsp */
    MEM_ON_CADENCE_DSP = 3,         /**< memory type cadence dsp */
    MEM_ON_CUDA_HOST = 4,           /**< memory type normal cuda */
    MEM_ON_CUDA_DEV = 5,            /**< memory type normal cuda */
    MEM_ON_MEMORY_TYPE_MAX = 6      /**< the max number of memory type */
} MemoryType;

/** A enum to represent pixel format */
typedef enum PixelFormat {
    M_PIX_FMT_GRAY8 = 1,             /**< gray8  */
    M_PIX_FMT_RGBA8888 = 2,          /**< rgba8888 */
    M_PIX_FMT_RGB888 = 3,            /**< rgb888 */
    M_PIX_FMT_RGB888_PLANAR = 4,     /**< rgb888 RRRRRR:GGGGGG:BBBBBB */
    M_PIX_FMT_BGRA8888 = 5,          /**< bgra8888 */
    M_PIX_FMT_BGR888 = 6,            /**< bgr888 */
    M_PIX_FMT_BGR888_PLANAR = 7,     /**< bgr888 BBBBBB:GGGGGG:RRRRRR */
    M_PIX_FMT_YUV420P = 8,           /**< yuv420p */
    M_PIX_FMT_NV12 = 9,              /**< YUV  4:2:0 YYYY:UV */
    M_PIX_FMT_NV21 = 10,             /**< YUV  4:2:0 YYYY:VU */
    M_PIX_FMT_GRAY32 = 11,           /**< gray32*/
    M_PIX_FMT_RGB323232 = 12,        /**< rgb323232 fp32*/
    M_PIX_FMT_RGB323232_PLANAR = 13, /**< rgb323232 fp32  RRRRRR:GGGGGG:BBBBBB*/
    M_PIX_FMT_BGR323232 = 14,        /**< bgr323232 fp32*/
    M_PIX_FMT_BGR323232_PLANAR = 15, /**< bgr323232 fp32 BBBBBB:GGGGGG:RRRRRR*/
    M_PIX_FMT_GRAY16 = 16,           /**< gray16*/
    M_PIX_FMT_RGB161616 = 17,        /**< rgb161616 fp16*/
    M_PIX_FMT_RGB161616_PLANAR = 18, /**< rgb161616 fp16  RRRRRR:GGGGGG:BBBBBB*/
    M_PIX_FMT_BGR161616 = 19,        /**< bgr161616 fp16*/
    M_PIX_FMT_BGR161616_PLANAR = 20, /**< bgr161616 fp16 BBBBBB:GGGGGG:RRRRRR*/
    M_PIX_FMT_FLOAT32C4 = 21,        /**< fp32 channel ==4 */
    M_PIX_FMT_NV12_DETACH = 22,      /**< Y/UV not Contiguous memory*/
    M_PIX_FMT_NV21_DETACH = 23,      /**< Y/UV not Contiguous memory*/
    M_PIX_FMT_YUYV = 24,             /**< YUV422 PACKED*/
    M_PIX_FMT_UYVY = 25,             /**< YUV422 PACKED*/
    M_PIX_FMT_YV12 = 26,             /**< YUV  4:2:0 YYYYYYYY:VVUU */
    M_PIX_FMT_YU12 = 27,             /**< YUV  4:2:0 YYYYYYYY:UUVV */
    M_PIX_FMT_MAX = 28               /**< pixel format is invalid */
} PixelFormat;

typedef enum MStatus {
    M_OK,
    M_FAILED,
    M_INVALID_ARG,
    M_FILE_NOT_FOUND,
    M_INVALID_FILE_FORMAT,
    M_INTERNAL_FAILED,
    M_OUT_OF_MEMORY,
    M_LICENSE_EXPIRE,
    M_UUID_MISMATCH,
    M_ONLINE_ACTIVATE_NO_NEED,
    M_ONLINE_ACTIVATE_FAIL,
    M_ONLINE_ACTIVATE_CODE_INVALID,
    M_ONLINE_ACTIVATE_CONNECT_FAIL,
    M_ONLINE_AUTH_TIME_OUT,
    M_LICENSE_ACTIVATIONS_RUN_OUT,
    M_MAX_STATUS
} MStatus;

} // __cplusplus
#endif // SIMPLE_BASE_COMMON_H_