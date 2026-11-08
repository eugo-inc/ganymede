/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zstd: cdict; calls ZSTD_createCDict(), ZSTD_getDictID_fromCDict(), ZSTD_getDictID_fromDict(), ZSTD_createCCtx().
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
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const size_t payload_len = sizeof(payload) - 1;

    ZSTD_CDict *cdict = ZSTD_createCDict(dict, dict_len, 5);
    assert(cdict != NULL);

    unsigned cdict_id = ZSTD_getDictID_fromCDict(cdict);
    unsigned raw_id   = ZSTD_getDictID_fromDict(dict, dict_len);
    assert(raw_id == 0);
    assert(cdict_id == 0);

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    assert(cctx != NULL);

    size_t cbuf_cap = ZSTD_compressBound(payload_len);
    char *cbuf = (char *)malloc(cbuf_cap);
    assert(cbuf != NULL);

    size_t csize = ZSTD_compress_usingCDict(cctx, cbuf, cbuf_cap,
                                            payload, payload_len, cdict);
    assert(!ZSTD_isError(csize));
    assert(csize > 0);

    /* Because the CDict is content-only, the frame is written with no dictID
     * tag: the frame header reports 0, matching the CDict. */
    unsigned frame_id = ZSTD_getDictID_fromFrame(cbuf, csize);
    assert(frame_id == 0);
    assert(frame_id == cdict_id);

    ZSTD_DDict *ddict = ZSTD_createDDict(dict, dict_len);
    assert(ddict != NULL);
    unsigned ddict_id = ZSTD_getDictID_fromDDict(ddict);
    assert(ddict_id == 0);
    assert(ddict_id == cdict_id);

    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    assert(dctx != NULL);

    char *dbuf = (char *)malloc(payload_len);
    assert(dbuf != NULL);

    size_t dsize = ZSTD_decompress_usingDDict(dctx, dbuf, payload_len,
                                              cbuf, csize, ddict);
    assert(!ZSTD_isError(dsize));
    assert(dsize == payload_len);
    assert(memcmp(dbuf, payload, payload_len) == 0);

    ZSTD_freeCDict(cdict);
    ZSTD_freeDDict(ddict);
    ZSTD_freeCCtx(cctx);
    ZSTD_freeDCtx(dctx);
    free(cbuf);
    free(dbuf);

    printf("zstd cdict OK: dictID=%u, %zu -> %zu -> %zu\n",
           cdict_id, payload_len, csize, payload_len);
    return 0;
}
