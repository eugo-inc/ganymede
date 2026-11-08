/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zstd: frame inspect; calls ZSTD_minCLevel(), ZSTD_maxCLevel(), ZSTD_compressBound(), ZSTD_compress().
 */

#include <zstd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int min_lvl = ZSTD_minCLevel();
    int max_lvl = ZSTD_maxCLevel();
    assert(min_lvl < 0);   /* negative levels exist (fastest) */
    assert(max_lvl >= 22); /* max documented level */

    static const char payload[] =
        "abcdefghijklmnopqrstuvwxyz0123456789"
        "abcdefghijklmnopqrstuvwxyz0123456789"
        "abcdefghijklmnopqrstuvwxyz0123456789";
    const size_t payload_len = sizeof(payload) - 1;

    size_t cbuf_cap = ZSTD_compressBound(payload_len);
    char *cbuf = (char *)malloc(cbuf_cap);
    assert(cbuf != NULL);

    size_t csize = ZSTD_compress(cbuf, cbuf_cap, payload, payload_len, 3);
    assert(!ZSTD_isError(csize));
    assert(csize > 0);

    /* ZSTD_getFrameContentSize: must report exactly payload_len, not the
     * sentinel ZSTD_CONTENTSIZE_UNKNOWN or ZSTD_CONTENTSIZE_ERROR. */
    unsigned long long advertised = ZSTD_getFrameContentSize(cbuf, csize);
    assert(advertised != ZSTD_CONTENTSIZE_UNKNOWN);
    assert(advertised != ZSTD_CONTENTSIZE_ERROR);
    assert(advertised == (unsigned long long)payload_len);

    /* ZSTD_findFrameCompressedSize: walk a buffer; should equal csize. */
    size_t frame_size = ZSTD_findFrameCompressedSize(cbuf, csize);
    assert(!ZSTD_isError(frame_size));
    assert(frame_size == csize);

    /* Corrupted/short input: must surface as an error, not silently succeed. */
    size_t short_rc = ZSTD_findFrameCompressedSize(cbuf, 2);
    assert(ZSTD_isError(short_rc));

    free(cbuf);

    printf("zstd frame-inspect OK: levels [%d,%d], advertised=%llu, frame_size=%zu\n",
           min_lvl, max_lvl, advertised, frame_size);
    return 0;
}
