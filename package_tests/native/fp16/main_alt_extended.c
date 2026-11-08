/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fp16: alt extended; calls fp16_alt_to_fp32_bits(), fp16_alt_to_fp32_value(), fp16_alt_from_fp32_value().
 */

#include <fp16.h>
#include <fp16/fp16.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* alt 0x7C00 encodes the FINITE value 2^16 = 65536.0
     * (whereas IEEE 0x7C00 encodes +inf — this is THE alt-vs-IEEE distinction). */
    const uint32_t bits_7C00 = fp16_alt_to_fp32_bits(UINT16_C(0x7C00));
    assert(bits_7C00 == UINT32_C(0x47800000));

    const float val_7C00 = fp16_alt_to_fp32_value(UINT16_C(0x7C00));
    assert(val_7C00 == 65536.0f);

    const uint16_t roundtrip_7C00 = fp16_alt_from_fp32_value(65536.0f);
    assert(roundtrip_7C00 == UINT16_C(0x7C00));

    /* Negative variant: alt 0xFC00 = -65536.0 (where IEEE would give -inf). */
    const float val_FC00 = fp16_alt_to_fp32_value(UINT16_C(0xFC00));
    assert(val_FC00 == -65536.0f);

    const uint16_t roundtrip_FC00 = fp16_alt_from_fp32_value(-65536.0f);
    assert(roundtrip_FC00 == UINT16_C(0xFC00));

    /* Maximum finite alt value: 0x7FFF = (1 + 1023/1024) * 2^16 = 131008.0
     * (where IEEE would treat 0x7FFF as NaN). */
    const uint32_t bits_max = fp16_alt_to_fp32_bits(UINT16_C(0x7FFF));
    assert(bits_max == UINT32_C(0x47FFE000));

    const float val_max = fp16_alt_to_fp32_value(UINT16_C(0x7FFF));
    assert(val_max == 131008.0f);

    const uint16_t roundtrip_max = fp16_alt_from_fp32_value(131008.0f);
    assert(roundtrip_max == UINT16_C(0x7FFF));

    /* Zero must still encode as zero in alt format. */
    const float zero_alt = fp16_alt_to_fp32_value(UINT16_C(0x0000));
    assert(zero_alt == 0.0f);
    assert(fp16_alt_to_fp32_bits(UINT16_C(0x0000)) == UINT32_C(0x00000000));

    /* Negative zero in alt -> sign-preserving. */
    assert(fp16_alt_to_fp32_bits(UINT16_C(0x8000)) == UINT32_C(0x80000000));

    printf("fp16 alt-format extended-range test OK\n");
    return 0;
}
