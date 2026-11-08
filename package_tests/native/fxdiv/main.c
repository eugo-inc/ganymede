/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fxdiv: core API; compiles against <fxdiv.h>.
 */

#include <fxdiv.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    const uint32_t divisor_value = 7u;
    const struct fxdiv_divisor_uint32_t divisor = fxdiv_init_uint32_t(divisor_value);

    /* Sweep a representative range — catches off-by-one in init's magic-number / shift state. */
    for (uint32_t n = 0; n < 1024u; ++n) {
        const struct fxdiv_result_uint32_t r = fxdiv_divide_uint32_t(n, divisor);
        assert(r.quotient == n / divisor_value);
        assert(r.remainder == n % divisor_value);
    }

    printf("fxdiv test OK\n");
    return 0;
}
