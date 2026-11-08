/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zstd: multiframe; calls ZSTD_compressBound(), ZSTD_compress(), ZSTD_isError(), ZSTD_findFrameCompressedSize().
 */

#include <zstd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char p1[] = "first frame payload AAAAAAAAAAAAAAAAAAAAAAAA";
    static const char p2[] = "second frame payload BBBBBBBBBBBBBBBBBBBBBBBB";
    static const char p3[] = "third frame payload CCCCCCCCCCCCCCCCCCCCCCCCC";
    const size_t l1 = sizeof(p1) - 1;
    const size_t l2 = sizeof(p2) - 1;
    const size_t l3 = sizeof(p3) - 1;
    const size_t total = l1 + l2 + l3;

    size_t bound = ZSTD_compressBound(l1) + ZSTD_compressBound(l2) + ZSTD_compressBound(l3);
    char *cbuf = (char *)malloc(bound);
    assert(cbuf != NULL);

    size_t c1 = ZSTD_compress(cbuf,           bound,           p1, l1, 3);
    assert(!ZSTD_isError(c1));
    size_t c2 = ZSTD_compress(cbuf + c1,      bound - c1,      p2, l2, 3);
    assert(!ZSTD_isError(c2));
    size_t c3 = ZSTD_compress(cbuf + c1 + c2, bound - c1 - c2, p3, l3, 3);
    assert(!ZSTD_isError(c3));

    size_t csize = c1 + c2 + c3;
    assert(csize > 0);

    /* Sanity: each individual frame boundary is recoverable. */
    size_t b1 = ZSTD_findFrameCompressedSize(cbuf,                csize);
    assert(!ZSTD_isError(b1));
    assert(b1 == c1);
    size_t b2 = ZSTD_findFrameCompressedSize(cbuf + c1,           csize - c1);
    assert(!ZSTD_isError(b2));
    assert(b2 == c2);
    size_t b3 = ZSTD_findFrameCompressedSize(cbuf + c1 + c2,      csize - c1 - c2);
    assert(!ZSTD_isError(b3));
    assert(b3 == c3);

    /* Now: decompress the WHOLE concatenated buffer through one DCtx loop.
     * The streaming decoder must transparently roll over frame boundaries. */
    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    assert(dctx != NULL);

    char *dbuf = (char *)malloc(total);
    assert(dbuf != NULL);

    ZSTD_inBuffer in   = { cbuf, csize, 0 };
    ZSTD_outBuffer out = { dbuf, total, 0 };

    size_t last_rc = (size_t)-1;
    while (in.pos < in.size) {
        last_rc = ZSTD_decompressStream(dctx, &out, &in);
        assert(!ZSTD_isError(last_rc));
    }
    /* Final return must indicate a clean frame boundary (no partial frame left). */
    assert(last_rc == 0);
    assert(out.pos == total);

    /* Byte-for-byte recovery of the concatenation. */
    assert(memcmp(dbuf,           p1, l1) == 0);
    assert(memcmp(dbuf + l1,      p2, l2) == 0);
    assert(memcmp(dbuf + l1 + l2, p3, l3) == 0);

    ZSTD_freeDCtx(dctx);
    free(cbuf);
    free(dbuf);

    printf("zstd multiframe OK: 3 frames, %zu+%zu+%zu compressed bytes -> %zu plaintext\n",
           c1, c2, c3, total);
    return 0;
}
