/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lzo: lzo1f; calls lzo_init(), lzo1f_1_compress(), lzo1f_decompress().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lzo/lzoconf.h>
#include <lzo/lzo1f.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define SRC_LEN 2048

int main(void) {
    assert(lzo_init() == LZO_E_OK);

    /* Mostly-redundant input with a small structured tail so the codec has
     * something non-trivial to encode. */
    unsigned char src[SRC_LEN];
    memset(src, 'Z', SRC_LEN);
    for (int i = 0; i < 64; ++i) {
        src[SRC_LEN - 64 + i] = (unsigned char)(i * 7 + 3);
    }

    /* lzo1f worst-case bound per the header: input + input/8 + 64 + 3. */
    unsigned char dst[SRC_LEN + SRC_LEN / 8 + 64 + 3];
    unsigned char roundtrip[SRC_LEN];

    unsigned char workmem[LZO1F_MEM_COMPRESS];

    lzo_uint dst_len = sizeof(dst);
    assert(lzo1f_1_compress(src, SRC_LEN, dst, &dst_len, workmem) == LZO_E_OK);
    /* Redundant prefix must compress noticeably. */
    assert(dst_len < SRC_LEN);

    lzo_uint rt_len = sizeof(roundtrip);
    assert(lzo1f_decompress(dst, dst_len, roundtrip, &rt_len, NULL) == LZO_E_OK);
    assert(rt_len == SRC_LEN);
    assert(memcmp(src, roundtrip, SRC_LEN) == 0);

    printf("lzo1f test OK: %lu -> %lu bytes\n",
           (unsigned long)SRC_LEN, (unsigned long)dst_len);
    return 0;
}
