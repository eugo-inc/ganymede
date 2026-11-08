/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for kleidi: bf16; calls kai_cast_bf16_f32(), kai_cast_f32_bf16(), kai_ext_sign_i8_i4().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <kai/kai_common.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* A floating-point value is exactly representable in bf16 iff its low
 * 16 mantissa bits (in IEEE-754 binary32 layout) are zero. */
static float f32_from_bits(uint32_t bits) {
    float f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

int main(void) {

    /* 0.0f, 1.0f, -1.0f, 2.0f all have zero low-16 mantissa bits. */
    const uint32_t exact_bits[] = {
        0x00000000u,  /* +0.0 */
        0x3F800000u,  /* 1.0 */
        0xBF800000u,  /* -1.0 */
        0x40000000u,  /* 2.0 */
        0x42F60000u,  /* 123.0 */
        0xC2F60000u,  /* -123.0 */
    };
    const size_t n = sizeof(exact_bits) / sizeof(exact_bits[0]);
    for (size_t i = 0; i < n; ++i) {
        const float original = f32_from_bits(exact_bits[i]);
        const uint16_t bf = kai_cast_bf16_f32(original);
        const float restored = kai_cast_f32_bf16(bf);

        uint32_t a, b;
        memcpy(&a, &original, sizeof(a));
        memcpy(&b, &restored, sizeof(b));
        assert(a == b);
    }

    {
        const uint32_t bits = 0x3F8ABCDEu;  /* approx 1.0838...; low half nonzero */
        const float original = f32_from_bits(bits);
        const uint16_t bf = kai_cast_bf16_f32(original);
#ifdef __ARM_FEATURE_BF16
        const uint32_t rne = bits + 0x7FFFu + ((bits >> 16) & 1u);
        assert(bf == (uint16_t)(rne >> 16));
#else
        assert(bf == (uint16_t)(bits >> 16));
#endif
    }

    /* --- int4 sign extension --- */

    /* Non-negative nibbles (high bit clear): identity. */
    assert(kai_ext_sign_i8_i4(0x0) == 0);
    assert(kai_ext_sign_i8_i4(0x1) == 1);
    assert(kai_ext_sign_i8_i4(0x7) == 7);

    /* Negative nibbles (high bit set): sign-extend to int8_t. */
    assert(kai_ext_sign_i8_i4(0x8) == -8);
    assert(kai_ext_sign_i8_i4(0x9) == -7);
    assert(kai_ext_sign_i8_i4(0xF) == -1);

    printf("kleidi bf16/int4 test OK\n");
    return 0;
}
