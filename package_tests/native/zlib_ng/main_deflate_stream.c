/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zlib_ng: deflate stream; calls zng_deflateInit2(), zng_deflate(), zng_deflateEnd(), zng_inflateInit2(), zng_inflate(), zng_inflateEnd().
 */

#include <zlib-ng.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* A 4 KiB highly-repetitive payload exercises the match finder. */
    enum { N = 4096 };
    uint8_t *input = (uint8_t *)malloc(N);
    assert(input != NULL);
    for (int i = 0; i < N; ++i) {
        input[i] = (uint8_t)("demozlibng"[i % 10]);
    }

    uint8_t compressed[N + 256];
    zng_stream zs;
    memset(&zs, 0, sizeof(zs));
    /* windowBits = -15 -> RAW deflate (no zlib wrapper, no gzip wrapper). */
    int32_t rc = zng_deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
    assert(rc == Z_OK);
    zs.next_in   = input;
    zs.avail_in  = N;
    zs.next_out  = compressed;
    zs.avail_out = sizeof(compressed);
    rc = zng_deflate(&zs, Z_FINISH);
    assert(rc == Z_STREAM_END);
    size_t compressed_len = zs.total_out;
    rc = zng_deflateEnd(&zs);
    assert(rc == Z_OK);

    /* Highly-repetitive input must shrink dramatically: sanity floor. */
    assert(compressed_len < (size_t)(N / 4));

    uint8_t *decompressed = (uint8_t *)calloc(N + 16, 1);
    assert(decompressed != NULL);
    zng_stream zi;
    memset(&zi, 0, sizeof(zi));
    rc = zng_inflateInit2(&zi, -15);
    assert(rc == Z_OK);
    zi.next_in   = compressed;
    zi.avail_in  = (uint32_t)compressed_len;
    zi.next_out  = decompressed;
    zi.avail_out = N + 16;
    rc = zng_inflate(&zi, Z_FINISH);
    assert(rc == Z_STREAM_END);
    assert(zi.total_out == N);
    rc = zng_inflateEnd(&zi);
    assert(rc == Z_OK);

    assert(memcmp(decompressed, input, N) == 0);

    printf("zlib_ng deflate-stream probe OK: %d -> %zu -> %d bytes (raw deflate)\n",
           N, compressed_len, N);
    free(input);
    free(decompressed);
    return 0;
}
