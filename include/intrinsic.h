#ifndef SIMPLE_BASE_MATH_H_
#define SIMPLE_BASE_MATH_H_

#include "common.h"

#include <stdio.h>
#ifdef USE_AVX
#include <immintrin.h>
#endif

SIMPLE_INLINE int divide_255(int value) {
    return (value + 1 + (value >> 8)) >> 8;
}

#ifdef USE_AVX
SIMPLE_INLINE __m256i _mm256_loadu2_m128i(__m128i const* __addr_hi, __m128i const* __addr_lo) {
    __m256i __v256 = _mm256_castsi128_si256(_mm_loadu_si128(__addr_lo));
    return _mm256_insertf128_si256(__v256, _mm_loadu_si128(__addr_hi), 1);
}

SIMPLE_INLINE void _mm256_storeu2_m128i(__m128i* __addr_hi, __m128i* __addr_lo, __m256i __a) {
    __m128i __v128;
    __v128 = _mm256_castsi256_si128(__a);
    _mm_storeu_si128(__addr_lo, __v128);
    __v128 = _mm256_extractf128_si256(__a, 1);
    _mm_storeu_si128(__addr_hi, __v128);
}

SIMPLE_INLINE void _mm_storeu_si32(void* mem_addr, __m128i a) {
    _mm_store_ss((float*)mem_addr, _mm_castsi128_ps(a));
}

SIMPLE_INLINE void _mm_storeu_si64(void* mem_addr, __m128i a) {
    _mm_store_sd((double*)mem_addr, _mm_castsi128_pd(a));
}

SIMPLE_INLINE void
_mm256_storeu_si256_planner(__m256i A, __m256i B, __m256i C, uint32_t stride, float* dst) {
    _mm256_storeu_si256((__m256i*)(dst), A);
    _mm256_storeu_si256((__m256i*)(dst + stride), B);
    _mm256_storeu_si256((__m256i*)(dst + 2 * stride), C);
}

// SIMPLE_INLINE void vld2_u8x16_avx(const uint8_t* src, __m128i* c0, __m128i* c1)
// {
//     const __m128i package_to_planner_c00 =
//         _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);
//     const __m128i package_to_planner_c01 =
//         _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 15, 13, 11, 9, 7, 5, 3, 1);

//     const __m128i src0_16 = _mm_loadu_si128((const __m128i*)(src + 0));
//     const __m128i u0_16   = _mm_shuffle_epi8(src0_16, package_to_planner_c00);
//     const __m128i v0_16   = _mm_shuffle_epi8(src0_16, package_to_planner_c01);

//     const __m128i src1_16 = _mm_loadu_si128((const __m128i*)(src + 16));
//     const __m128i u1_16   = _mm_shuffle_epi8(src1_16, package_to_planner_c00);
//     const __m128i v1_16   = _mm_shuffle_epi8(src1_16, package_to_planner_c01);

//     *c0 = _mm_or_si128(u0_16, u1_16);
//     *c1 = _mm_or_si128(u1_16, v1_16);
// }

// clang-format off
SIMPLE_INLINE void vld2_u8x16_avx(const uint8_t* src, __m128i* c0, __m128i* c1)
{
    const __m128i package_to_planner_c00 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);
    const __m128i package_to_planner_c01 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 15, 13, 11, 9, 7, 5, 3, 1);

    const __m128i package_to_planner_c10 =
        _mm_set_epi8(14, 12, 10, 8, 6, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_c11 =
        _mm_set_epi8(15, 13, 11, 9, 7, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m128i src0_16 = _mm_loadu_si128((const __m128i*)(src + 0));
    const __m128i u0_16   = _mm_shuffle_epi8(src0_16, package_to_planner_c00);
    const __m128i v0_16   = _mm_shuffle_epi8(src0_16, package_to_planner_c01);
    const __m128i src1_16 = _mm_loadu_si128((const __m128i*)(src + 16));
    const __m128i u1_16   = _mm_shuffle_epi8(src1_16, package_to_planner_c10);
    const __m128i v1_16   = _mm_shuffle_epi8(src1_16, package_to_planner_c11);

    *c0 = _mm_or_si128(u0_16, u1_16);
    *c1 = _mm_or_si128(v0_16, v1_16);
}

SIMPLE_INLINE void vld3_u8x16_avx(const uint8_t* src, __m128i* c0, __m128i* c1, __m128i* c2)
{
    const __m128i package_to_planner_bshuff0 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);
    const __m128i package_to_planner_gshuff0 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);
    const __m128i package_to_planner_rshuff0 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);
    const __m128i package_to_planner_bshuff1 =
        _mm_set_epi8(-1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_gshuff1 =
        _mm_set_epi8(-1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_rshuff1 =
        _mm_set_epi8(-1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_bshuff2 =
        _mm_set_epi8(13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_gshuff2 =
        _mm_set_epi8(14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m128i package_to_planner_rshuff2 =
        _mm_set_epi8(15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    const __m128i bgr0 = _mm_loadu_si128((const __m128i*)(src + 0));
    const __m128i bgr1 = _mm_loadu_si128((const __m128i*)(src + 16));
    const __m128i bgr2 = _mm_loadu_si128((const __m128i*)(src + 32));
    const __m128i b0   = _mm_shuffle_epi8(bgr0, package_to_planner_bshuff0);
    const __m128i g0   = _mm_shuffle_epi8(bgr0, package_to_planner_gshuff0);
    const __m128i r0   = _mm_shuffle_epi8(bgr0, package_to_planner_rshuff0);

    const __m128i b1 = _mm_shuffle_epi8(bgr1, package_to_planner_bshuff1);
    const __m128i g1 = _mm_shuffle_epi8(bgr1, package_to_planner_gshuff1);
    const __m128i r1 = _mm_shuffle_epi8(bgr1, package_to_planner_rshuff1);

    const __m128i b2 = _mm_shuffle_epi8(bgr2, package_to_planner_bshuff2);
    const __m128i g2 = _mm_shuffle_epi8(bgr2, package_to_planner_gshuff2);
    const __m128i r2 = _mm_shuffle_epi8(bgr2, package_to_planner_rshuff2);

    *c0 = _mm_or_si128(_mm_or_si128(b0, b1), b2);
    *c1 = _mm_or_si128(_mm_or_si128(g0, g1), g2);
    *c2 = _mm_or_si128(_mm_or_si128(r0, r1), r2);
}

SIMPLE_INLINE void vst3_u8x16_avx(__m128i c0, __m128i c1, __m128i c2, uint8_t* dst)
{
    __m128i planner_to_b0_shuff =
        _mm_set_epi8(5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0);
    __m128i planner_to_b1_shuff =
        _mm_set_epi8(-1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1);
    __m128i planner_to_b2_shuff =
        _mm_set_epi8(-1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1);
    __m128i planner_to_g0_shuff =
        _mm_set_epi8(-1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1);
    __m128i planner_to_g1_shuff =
        _mm_set_epi8(10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5);
    __m128i planner_to_g2_shuff =
        _mm_set_epi8(-1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1);
    __m128i planner_to_r0_shuff =
        _mm_set_epi8(-1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1);
    __m128i planner_to_r1_shuff =
        _mm_set_epi8(-1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1);
    __m128i planner_to_r2_shuff =
        _mm_set_epi8(15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10);

    __m128i B0 = _mm_shuffle_epi8(c0, planner_to_b0_shuff);
    __m128i B1 = _mm_shuffle_epi8(c0, planner_to_b1_shuff);
    __m128i B2 = _mm_shuffle_epi8(c0, planner_to_b2_shuff);
    __m128i G0 = _mm_shuffle_epi8(c1, planner_to_g0_shuff);
    __m128i G1 = _mm_shuffle_epi8(c1, planner_to_g1_shuff);
    __m128i G2 = _mm_shuffle_epi8(c1, planner_to_g2_shuff);
    __m128i R0 = _mm_shuffle_epi8(c2, planner_to_r0_shuff);
    __m128i R1 = _mm_shuffle_epi8(c2, planner_to_r1_shuff);
    __m128i R2 = _mm_shuffle_epi8(c2, planner_to_r2_shuff);

    __m128i bgr0 = _mm_or_si128(_mm_or_si128(R0, G0), B0);
    __m128i bgr1 = _mm_or_si128(_mm_or_si128(R1, G1), B1);
    __m128i bgr2 = _mm_or_si128(_mm_or_si128(R2, G2), B2);

    _mm_storeu_si128((__m128i*)(dst + 0), bgr0);
    _mm_storeu_si128((__m128i*)(dst + 16), bgr1);
    _mm_storeu_si128((__m128i*)(dst + 32), bgr2);
}

SIMPLE_INLINE void vst3_f32x8_avx(__m256 vec_c0, __m256 vec_c1, __m256 vec_c2, float* dst_f32)
{
    __m256i mask_c0_0 = _mm256_set_epi8(7, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0, 7,
                                        6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0);
    __m256i mask_c0_1 =
        _mm256_set_epi8(-1, -1, -1, -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1);
    __m256i mask_c0_2 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1);
    __m256i mask_c1_0 = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1,
                                        -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1);
    __m256i mask_c1_1 = _mm256_set_epi8(11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4,
                                        11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4);
    __m256i mask_c1_2 =
        _mm256_set_epi8(-1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1);
    __m256i mask_c2_0 = _mm256_set_epi8(-1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1,
                                        -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1);
    __m256i mask_c2_1 = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1,
                                        -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1);
    __m256i mask_c2_2 =
        _mm256_set_epi8(15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8, 15, 14, 13,
                        12, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8);

    __m256i b0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), mask_c0_0);
    __m256i b1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), mask_c0_1);
    __m256i b2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), mask_c0_2);

    __m256i g0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), mask_c1_0);
    __m256i g1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), mask_c1_1);
    __m256i g2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), mask_c1_2);

    __m256i r0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), mask_c2_0);
    __m256i r1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), mask_c2_1);
    __m256i r2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), mask_c2_2);

    __m256i bgr0 = _mm256_or_si256(_mm256_or_si256(b0, g0), r0);
    __m256i bgr1 = _mm256_or_si256(_mm256_or_si256(b1, g1), r1);
    __m256i bgr2 = _mm256_or_si256(_mm256_or_si256(b2, g2), r2);

    _mm256_storeu_si256((__m256i*)(dst_f32 + 0), _mm256_permute2x128_si256(bgr0, bgr1, 0b00100000));
    _mm256_storeu_si256((__m256i*)(dst_f32 + 8), _mm256_permute2x128_si256(bgr2, bgr0, 0b00110000));
    _mm256_storeu_si256((__m256i*)(dst_f32 + 16),
                        _mm256_permute2x128_si256(bgr1, bgr2, 0b00110001));
}

SIMPLE_INLINE void vld3_u8x32_avx(const uint8_t* src, __m256i* c0, __m256i* c1, __m256i* c2)
{
    const __m256i mask_c0_0 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);
    const __m256i mask_c0_1 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);
    const __m256i mask_c0_2 =
        _mm256_set_epi8(13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4,
                        1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m256i mask_c1_0 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);
    const __m256i mask_c1_1 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);
    const __m256i mask_c1_2 =
        _mm256_set_epi8(14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5,
                        2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    const __m256i mask_c2_0 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1,
                        -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);
    const __m256i mask_c2_1 =
        _mm256_set_epi8(-1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                        -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);
    const __m256i mask_c2_2 =
        _mm256_set_epi8(15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3,
                        0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i bgr_0 = _mm256_loadu2_m128i((__m128i const*)(src + 48), (__m128i const*)(src + 0));
    __m256i bgr_1 = _mm256_loadu2_m128i((__m128i const*)(src + 64), (__m128i const*)(src + 16));
    __m256i bgr_2 = _mm256_loadu2_m128i((__m128i const*)(src + 80), (__m128i const*)(src + 32));

    __m256i b0 = _mm256_shuffle_epi8(bgr_0, mask_c0_0);
    __m256i b1 = _mm256_shuffle_epi8(bgr_1, mask_c0_1);
    __m256i b2 = _mm256_shuffle_epi8(bgr_2, mask_c0_2);

    __m256i g0 = _mm256_shuffle_epi8(bgr_0, mask_c1_0);
    __m256i g1 = _mm256_shuffle_epi8(bgr_1, mask_c1_1);
    __m256i g2 = _mm256_shuffle_epi8(bgr_2, mask_c1_2);

    __m256i r0 = _mm256_shuffle_epi8(bgr_0, mask_c2_0);
    __m256i r1 = _mm256_shuffle_epi8(bgr_1, mask_c2_1);
    __m256i r2 = _mm256_shuffle_epi8(bgr_2, mask_c2_2);

    *c0 = _mm256_or_si256(_mm256_or_si256(b0, b1), b2);
    *c1 = _mm256_or_si256(_mm256_or_si256(g0, g1), g2);
    *c2 = _mm256_or_si256(_mm256_or_si256(r0, r1), r2);
}

SIMPLE_INLINE void vst3_u8x32_avx(__m256i c0, __m256i c1, __m256i c2, uint8_t* dst)
{
    const __m256i s_pl_to_pa_b0_shuff =
        _mm256_set_epi8(5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, 5, -1, -1, 4, -1,
                        -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0);
    const __m256i s_pl_to_pa_b1_shuff =
        _mm256_set_epi8(-1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, -1, 10, -1, -1,
                        9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1);
    const __m256i s_pl_to_pa_b2_shuff =
        _mm256_set_epi8(-1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, -1, 15,
                        -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1);
    const __m256i s_pl_to_pa_g0_shuff =
        _mm256_set_epi8(-1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1, -1, 4, -1,
                        -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1);
    const __m256i s_pl_to_pa_g1_shuff =
        _mm256_set_epi8(10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, 10, -1, -1, 9,
                        -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5);
    const __m256i s_pl_to_pa_g2_shuff =
        _mm256_set_epi8(-1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, -1, 15, -1,
                        -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1);
    const __m256i s_pl_to_pa_r0_shuff =
        _mm256_set_epi8(-1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1, -1, 4, -1, -1, 3,
                        -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1);
    const __m256i s_pl_to_pa_r1_shuff =
        _mm256_set_epi8(-1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1, -1, -1, 9, -1,
                        -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1);
    const __m256i s_pl_to_pa_r2_shuff =
        _mm256_set_epi8(15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10, 15, -1, -1,
                        14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10);
    __m256i bgr0, bgr1, bgr2;
    __m256i B0 = _mm256_shuffle_epi8(c0, s_pl_to_pa_b0_shuff);
    __m256i B1 = _mm256_shuffle_epi8(c0, s_pl_to_pa_b1_shuff);
    __m256i B2 = _mm256_shuffle_epi8(c0, s_pl_to_pa_b2_shuff);

    __m256i G0 = _mm256_shuffle_epi8(c1, s_pl_to_pa_g0_shuff);
    __m256i G1 = _mm256_shuffle_epi8(c1, s_pl_to_pa_g1_shuff);
    __m256i G2 = _mm256_shuffle_epi8(c1, s_pl_to_pa_g2_shuff);

    __m256i R0 = _mm256_shuffle_epi8(c2, s_pl_to_pa_r0_shuff);
    __m256i R1 = _mm256_shuffle_epi8(c2, s_pl_to_pa_r1_shuff);
    __m256i R2 = _mm256_shuffle_epi8(c2, s_pl_to_pa_r2_shuff);

    bgr0 = _mm256_or_si256(_mm256_or_si256(R0, G0), B0);
    bgr1 = _mm256_or_si256(_mm256_or_si256(R1, G1), B1);
    bgr2 = _mm256_or_si256(_mm256_or_si256(R2, G2), B2);

    // TODO May optimize store here later!
    // _mm_storeu_si128((__m128i*)(dst +  0), _mm256_castsi256_si128(bgr0));
    // _mm_storeu_si128((__m128i*)(dst + 16), _mm256_castsi256_si128(bgr1));
    // _mm_storeu_si128((__m128i*)(dst + 32), _mm256_castsi256_si128(bgr2));

    // _mm_storeu_si128((__m128i*)(dst + 48), _mm256_extracti128_si256(bgr0, 1));
    // _mm_storeu_si128((__m128i*)(dst + 64), _mm256_extracti128_si256(bgr1, 1));
    // _mm_storeu_si128((__m128i*)(dst + 80), _mm256_extracti128_si256(bgr2, 1));

    _mm256_storeu_si256((__m256i*)(dst + 0), _mm256_permute2x128_si256(bgr0, bgr1, 0b00100000));
    _mm256_storeu_si256((__m256i*)(dst + 32), _mm256_permute2x128_si256(bgr2, bgr0, 0b00110000));
    _mm256_storeu_si256((__m256i*)(dst + 64), _mm256_permute2x128_si256(bgr1, bgr2, 0b00110001));
}
#endif // USE_AVX

#endif // SIMPLE_BASE_MATH_H_