/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zstd: streaming; calls ZSTD_createCCtx(), ZSTD_CCtx_setParameter(), ZSTD_isError(), ZSTD_compressBound().
 */

#include <zstd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const size_t payload_len = 4096;
    char *payload = (char *)malloc(payload_len);
    assert(payload != NULL);
    for (size_t i = 0; i < payload_len; ++i) {
        payload[i] = (char)('A' + (i % 26));
    }

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    assert(cctx != NULL);

    size_t set_rc = ZSTD_CCtx_setParameter(cctx, ZSTD_c_compressionLevel, 5);
    assert(!ZSTD_isError(set_rc));

    size_t cbuf_cap = ZSTD_compressBound(payload_len);
    assert(cbuf_cap > 0);
    char *cbuf = (char *)malloc(cbuf_cap);
    assert(cbuf != NULL);

    ZSTD_inBuffer in = { payload, payload_len, 0 };
    ZSTD_outBuffer out = { cbuf, cbuf_cap, 0 };

    /* Drive the stream to completion with ZSTD_e_end. */
    size_t remaining;
    do {
        remaining = ZSTD_compressStream2(cctx, &out, &in, ZSTD_e_end);
        assert(!ZSTD_isError(remaining));
    } while (remaining != 0);
    assert(in.pos == in.size);
    size_t csize = out.pos;
    assert(csize > 0);
    assert(csize < payload_len); /* structured payload must actually shrink */

    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    assert(dctx != NULL);

    char *dbuf = (char *)malloc(payload_len);
    assert(dbuf != NULL);

    ZSTD_inBuffer din = { cbuf, csize, 0 };
    ZSTD_outBuffer dout = { dbuf, payload_len, 0 };

    /* A single decompressStream call must consume the whole frame here
     * (payload fits in dout); rc == 0 means frame complete. */
    size_t drc = ZSTD_decompressStream(dctx, &dout, &din);
    assert(!ZSTD_isError(drc));
    assert(drc == 0);
    assert(dout.pos == payload_len);
    assert(memcmp(dbuf, payload, payload_len) == 0);

    ZSTD_freeCCtx(cctx);
    ZSTD_freeDCtx(dctx);
    free(payload);
    free(cbuf);
    free(dbuf);

    printf("zstd streaming OK: %zu -> %zu -> %zu\n", payload_len, csize, payload_len);
    return 0;
}
