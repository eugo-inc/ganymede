/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zstd: dict; calls ZSTD_createCCtx(), ZSTD_compressBound(), ZSTD_compress_usingDict(), ZSTD_isError().
 */

#include <zstd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char dict[] =
        "the quick brown fox jumps over the lazy dog "
        "pack my box with five dozen liquor jugs "
        "sphinx of black quartz judge my vow ";
    const size_t dict_len = sizeof(dict) - 1;

    static const char payload[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const size_t payload_len = sizeof(payload) - 1;

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    assert(cctx != NULL);

    size_t cbuf_cap = ZSTD_compressBound(payload_len);
    char *cbuf = (char *)malloc(cbuf_cap);
    assert(cbuf != NULL);

    size_t csize = ZSTD_compress_usingDict(cctx, cbuf, cbuf_cap,
                                           payload, payload_len,
                                           dict, dict_len, 3);
    assert(!ZSTD_isError(csize));
    assert(csize > 0);

    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    assert(dctx != NULL);

    char *dbuf = (char *)malloc(payload_len);
    assert(dbuf != NULL);

    size_t dsize = ZSTD_decompress_usingDict(dctx, dbuf, payload_len,
                                             cbuf, csize,
                                             dict, dict_len);
    assert(!ZSTD_isError(dsize));
    assert(dsize == payload_len);
    assert(memcmp(dbuf, payload, payload_len) == 0);

    /* Same frame, decompressed WITHOUT the dictionary: must fail, because its matches refer
     * back into dictionary content the decoder does not have. */
    char *dbuf2 = (char *)malloc(payload_len);
    assert(dbuf2 != NULL);
    size_t fail_rc = ZSTD_decompress(dbuf2, payload_len, cbuf, csize);
    assert(ZSTD_isError(fail_rc));

    ZSTD_freeCCtx(cctx);
    ZSTD_freeDCtx(dctx);
    free(cbuf);
    free(dbuf);
    free(dbuf2);

    printf("zstd dict OK: %zu -> %zu -> %zu (no-dict decompress correctly rejected)\n",
           payload_len, csize, payload_len);
    return 0;
}
