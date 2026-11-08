/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fxdiv: round down; compiles against <fxdiv.h>.
 */

#include <fxdiv.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    const uint32_t granularity_value = 13u;
    const struct fxdiv_divisor_uint32_t granularity = fxdiv_init_uint32_t(granularity_value);

    for (uint32_t n = 0; n < 2048u; ++n) {
        const uint32_t q = fxdiv_quotient_uint32_t(n, granularity);
        const uint32_t r = fxdiv_remainder_uint32_t(n, granularity);
        const uint32_t rd = fxdiv_round_down_uint32_t(n, granularity);

        assert(q == n / granularity_value);
        assert(r == n % granularity_value);
        /* round_down(n, g) == g * floor(n / g) — must equal n - (n % g). */
        assert(rd == n - (n % granularity_value));
        /* Consistency between the three: q*g + r == n, rd + r == n. */
        assert(q * granularity_value + r == n);
        assert(rd + r == n);
    }

    /* Power-of-two granularity stresses the s1==0 / s2==0 branch in init. */
    const uint32_t pow2_value = 64u;
    const struct fxdiv_divisor_uint32_t pow2 = fxdiv_init_uint32_t(pow2_value);
    for (uint32_t n = 0; n < 512u; ++n) {
        const uint32_t rd = fxdiv_round_down_uint32_t(n, pow2);
        assert(rd == (n & ~(pow2_value - 1u)));
    }

    printf("fxdiv quotient/remainder/round_down test OK\n");
    return 0;
}
