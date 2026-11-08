/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libunistring: conv; calls u8_to_u32(), u32_to_u8().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <unistr.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* "héllo" in UTF-8: 'h', U+00E9 (0xC3 0xA9), 'l', 'l', 'o' — 6 bytes, 5 code points. */
    static const uint8_t utf8_in[] = {0x68, 0xC3, 0xA9, 0x6C, 0x6C, 0x6F};
    static const uint32_t expected_u32[] = {0x68, 0xE9, 0x6C, 0x6C, 0x6F};

    /* UTF-8 -> UTF-32 with library-allocated buffer (resultbuf=NULL, lengthp tracks size). */
    size_t out_len = 0;
    uint32_t *u32 = u8_to_u32(utf8_in, sizeof(utf8_in), NULL, &out_len);
    assert(u32 != NULL);
    assert(out_len == 5);
    for (size_t i = 0; i < out_len; ++i) {
        assert(u32[i] == expected_u32[i]);
    }

    size_t back_len = 0;
    uint8_t *back = u32_to_u8(u32, out_len, NULL, &back_len);
    assert(back != NULL);
    assert(back_len == sizeof(utf8_in));
    assert(memcmp(back, utf8_in, sizeof(utf8_in)) == 0);

    free(u32);
    free(back);

    printf("libunistring conv test OK\n");
    return 0;
}
