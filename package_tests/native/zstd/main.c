/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zstd: core API; calls ZSTD_compressBound(), ZSTD_compress(), ZSTD_isError(), ZSTD_decompress().
 */

#include <zstd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char payload[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const size_t payload_len = sizeof(payload) - 1;

    size_t cbuf_cap = ZSTD_compressBound(payload_len);
    assert(cbuf_cap > 0);
    char cbuf[512];
    assert(cbuf_cap <= sizeof(cbuf));

    size_t csize = ZSTD_compress(cbuf, sizeof(cbuf), payload, payload_len, 3);
    assert(!ZSTD_isError(csize));
    assert(csize > 0);

    char dbuf[512];
    size_t dsize = ZSTD_decompress(dbuf, sizeof(dbuf), cbuf, csize);
    assert(!ZSTD_isError(dsize));
    assert(dsize == payload_len);
    assert(memcmp(dbuf, payload, payload_len) == 0);

    printf("zstd test OK: %s\n", ZSTD_versionString());
    return 0;
}
