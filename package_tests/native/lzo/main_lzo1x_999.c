/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lzo: lzo1x 999; calls lzo_init(), lzo1x_999_compress(), lzo1x_decompress_safe().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lzo/lzoconf.h>
#include <lzo/lzo1x.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define SRC_LEN 4096

int main(void) {
    assert(lzo_init() == LZO_E_OK);

    /* Repeating short pattern — exercises high-compression match search. */
    unsigned char src[SRC_LEN];
    for (int i = 0; i < SRC_LEN; ++i) {
        src[i] = (unsigned char)("demo-lzo-999-pattern-"[i % 21]);
    }

    unsigned char dst[SRC_LEN + SRC_LEN / 16 + 64 + 3];
    unsigned char roundtrip[SRC_LEN];

    unsigned char workmem[LZO1X_999_MEM_COMPRESS];

    lzo_uint dst_len = sizeof(dst);
    assert(lzo1x_999_compress(src, SRC_LEN, dst, &dst_len, workmem) == LZO_E_OK);
    /* Highly repetitive input should compress aggressively under -999. */
    assert(dst_len < SRC_LEN / 2);

    lzo_uint rt_len = sizeof(roundtrip);
    assert(lzo1x_decompress_safe(dst, dst_len, roundtrip, &rt_len, NULL) == LZO_E_OK);
    assert(rt_len == SRC_LEN);
    assert(memcmp(src, roundtrip, SRC_LEN) == 0);

    if (dst_len > 4) {
        memset(roundtrip, 0xCC, sizeof(roundtrip));
        lzo_uint trunc_len = sizeof(roundtrip);
        int rc = lzo1x_decompress_safe(dst, dst_len - 2, roundtrip, &trunc_len, NULL);
        assert(rc != LZO_E_OK);
    }

    printf("lzo1x_999 + safe-decompress test OK: %lu -> %lu bytes\n",
           (unsigned long)SRC_LEN, (unsigned long)dst_len);
    return 0;
}
