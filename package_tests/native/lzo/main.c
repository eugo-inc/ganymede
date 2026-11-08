/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lzo: core API; calls lzo_init(), lzo1x_1_compress(), lzo1x_decompress(), lzo_version_string().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define SRC_LEN 1024

int main(void) {
    assert(lzo_init() == LZO_E_OK);

    /* Highly redundant input so LZO actually finds something to compress. */
    unsigned char src[SRC_LEN];
    memset(src, 'A', SRC_LEN);

    /* Worst-case bound per lzo1x docs: input + input/16 + 64 + 3. */
    unsigned char dst[SRC_LEN + SRC_LEN / 16 + 64 + 3];
    unsigned char roundtrip[SRC_LEN];

    /* lzo1x_1_compress needs LZO1X_1_MEM_COMPRESS bytes of working memory. */
    unsigned char workmem[LZO1X_1_MEM_COMPRESS];

    lzo_uint dst_len = sizeof(dst);
    assert(lzo1x_1_compress(src, SRC_LEN, dst, &dst_len, workmem) == LZO_E_OK);
    assert(dst_len < SRC_LEN);  /* Should compress redundant data. */

    lzo_uint rt_len = sizeof(roundtrip);
    assert(lzo1x_decompress(dst, dst_len, roundtrip, &rt_len, NULL) == LZO_E_OK);
    assert(rt_len == SRC_LEN);
    assert(memcmp(src, roundtrip, SRC_LEN) == 0);

    printf("lzo test OK: %s\n", lzo_version_string());
    return 0;
}
