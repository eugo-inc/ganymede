/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fxdiv: uint64; compiles against <fxdiv.h>.
 */

#include <fxdiv.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* A divisor that doesn't fit in 32 bits — forces the 64-bit code path. */
    const uint64_t divisor_value = (UINT64_C(1) << 33) | UINT64_C(12345);
    const struct fxdiv_divisor_uint64_t divisor = fxdiv_init_uint64_t(divisor_value);

    const uint64_t dividends[] = {
        UINT64_C(0),
        UINT64_C(1),
        divisor_value - 1u,
        divisor_value,
        divisor_value + 1u,
        UINT64_C(1) << 32,
        UINT64_C(1) << 48,
        UINT64_C(0xFFFFFFFFFFFFFFFF),
    };
    const size_t n_dividends = sizeof(dividends) / sizeof(dividends[0]);

    for (size_t i = 0; i < n_dividends; ++i) {
        const uint64_t n = dividends[i];
        const struct fxdiv_result_uint64_t r = fxdiv_divide_uint64_t(n, divisor);
        assert(r.quotient == n / divisor_value);
        assert(r.remainder == n % divisor_value);
    }

    /* Also exercise divisor == 1 — special-cased branch in init. */
    const struct fxdiv_divisor_uint64_t one = fxdiv_init_uint64_t(UINT64_C(1));
    const struct fxdiv_result_uint64_t r1 = fxdiv_divide_uint64_t(UINT64_C(0xDEADBEEFCAFEBABE), one);
    assert(r1.quotient == UINT64_C(0xDEADBEEFCAFEBABE));
    assert(r1.remainder == UINT64_C(0));

    printf("fxdiv uint64 test OK\n");
    return 0;
}
