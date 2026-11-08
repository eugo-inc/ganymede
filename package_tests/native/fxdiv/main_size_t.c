/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fxdiv: size t; compiles against <fxdiv.h>.
 */

#include <fxdiv.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    assert(SIZE_MAX == UINT32_MAX || SIZE_MAX == UINT64_MAX);

    const size_t divisor_value = (size_t) 17;
    const struct fxdiv_divisor_size_t divisor = fxdiv_init_size_t(divisor_value);

    /* The wrapper must preserve `value` exactly across the platform dispatch. */
    assert(divisor.value == divisor_value);

    for (size_t n = 0; n < 4096; ++n) {
        const struct fxdiv_result_size_t r = fxdiv_divide_size_t(n, divisor);
        assert(r.quotient == n / divisor_value);
        assert(r.remainder == n % divisor_value);
    }

    /* A larger divisor that would overflow 32-bit on 64-bit platforms. */
    if (SIZE_MAX == UINT64_MAX) {
        const size_t big_divisor = ((size_t) 1u << 40) + (size_t) 7u;
        const struct fxdiv_divisor_size_t big = fxdiv_init_size_t(big_divisor);
        const size_t n = ((size_t) 1u << 50) + (size_t) 999u;
        const struct fxdiv_result_size_t r = fxdiv_divide_size_t(n, big);
        assert(r.quotient == n / big_divisor);
        assert(r.remainder == n % big_divisor);
    }

    printf("fxdiv size_t test OK\n");
    return 0;
}
