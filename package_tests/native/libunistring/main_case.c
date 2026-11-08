/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libunistring: case; calls u8_toupper(), u8_tolower().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <unicase.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* "héllo" in UTF-8 -> "HÉLLO" (U+00C9 = 0xC3 0x89). */
    static const uint8_t in[] = {0x68, 0xC3, 0xA9, 0x6C, 0x6C, 0x6F};
    static const uint8_t expected_upper[] = {0x48, 0xC3, 0x89, 0x4C, 0x4C, 0x4F};

    size_t out_len = 0;
    uint8_t *upper = u8_toupper(in, sizeof(in), "C", NULL, NULL, &out_len);
    assert(upper != NULL);
    assert(out_len == sizeof(expected_upper));
    assert(memcmp(upper, expected_upper, out_len) == 0);

    size_t lower_len = 0;
    uint8_t *lower = u8_tolower(upper, out_len, "C", NULL, NULL, &lower_len);
    assert(lower != NULL);
    assert(lower_len == sizeof(in));
    assert(memcmp(lower, in, lower_len) == 0);

    free(upper);
    free(lower);

    printf("libunistring case test OK\n");
    return 0;
}
