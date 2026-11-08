/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zlib_ng: native core API; calls zng_compressBound(), zng_compress(), zng_uncompress(), zlibng_version().
 */

#include <zlib-ng.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *input = "the quick brown fox jumps over the lazy dog";
    const size_t input_len = strlen(input);

    /* The destination buffer must hold at least zng_compressBound(sourceLen) bytes. */
    uint8_t compressed[256];
    assert(zng_compressBound(input_len) <= sizeof(compressed));
    size_t compressed_len = sizeof(compressed);
    int32_t rc = zng_compress(compressed, &compressed_len, (const uint8_t *)input, input_len);
    assert(rc == Z_OK);
    assert(compressed_len < input_len + 64);

    /* Decompress into a buffer one byte larger than needed, so the length check is strict. */
    uint8_t decompressed[256] = {0};
    size_t decompressed_len = sizeof(decompressed) - 1;
    rc = zng_uncompress(decompressed, &decompressed_len, compressed, compressed_len);
    assert(rc == Z_OK);
    assert(decompressed_len == input_len);
    assert(memcmp(decompressed, input, input_len) == 0);

    printf("zlib_ng test OK: %s\n", zlibng_version());
    return 0;
}
