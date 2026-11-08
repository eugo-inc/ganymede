/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libbsd: core API; calls strlcpy(), strlcat(), arc4random_uniform().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <bsd/bsd.h>
#include <bsd/string.h>
#include <bsd/stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* strlcpy: copies + NUL-terminates; returns total length of src (BSD contract). */
    char buf[16];
    size_t n = strlcpy(buf, "hello", sizeof(buf));
    assert(n == 5);
    assert(strcmp(buf, "hello") == 0);

    /* strlcat: appends + NUL-terminates; returns total length it tried to create. */
    n = strlcat(buf, " bsd", sizeof(buf));
    assert(n == 9);
    assert(strcmp(buf, "hello bsd") == 0);

    /* arc4random_uniform(N) returns a value in [0, N). Sample 64 times — must always stay in range. */
    for (int i = 0; i < 64; ++i) {
        uint32_t r = arc4random_uniform(10);
        assert(r < 10);
    }

    printf("libbsd test OK\n");
    return 0;
}
