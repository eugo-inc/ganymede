/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for bzip2: corrupt; calls BZ2_bzBuffToBuffDecompress(), BZ2_bzBuffToBuffCompress().
 */

#include <bzlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Case 1: random bytes (no BZh magic) — must yield BZ_DATA_ERROR_MAGIC. */
    char garbage[256];
    for (int i = 0; i < (int)sizeof(garbage); ++i) {
        garbage[i] = (char)(i ^ 0x5A);
    }
    char out[4096];
    unsigned int out_len = sizeof(out);
    int rc = BZ2_bzBuffToBuffDecompress(
        out, &out_len, garbage, (unsigned int)sizeof(garbage),
        /* small */ 0, /* verbosity */ 0);
    assert(rc == BZ_DATA_ERROR_MAGIC);

    static const char payload[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const unsigned int payload_len = (unsigned int)sizeof(payload) - 1;
    char compressed[4096];
    unsigned int compressed_len = sizeof(compressed);
    rc = BZ2_bzBuffToBuffCompress(
        compressed, &compressed_len, (char *)payload, payload_len,
        /* blockSize100k */ 9, /* verbosity */ 0, /* workFactor */ 0);
    assert(rc == BZ_OK);
    assert(compressed_len > 8);

    unsigned int truncated_len = compressed_len - 8;
    out_len = sizeof(out);
    rc = BZ2_bzBuffToBuffDecompress(
        out, &out_len, compressed, truncated_len,
        /* small */ 0, /* verbosity */ 0);
    /* Acceptable rejections: unexpected EOF, generic data error, or output-too-small.
     * The critical assertion is that the library does NOT report BZ_OK on a
     * corrupted stream. */
    assert(rc != BZ_OK);
    assert(rc == BZ_UNEXPECTED_EOF || rc == BZ_DATA_ERROR || rc == BZ_OUTBUFF_FULL);

    /* Case 3: flip a byte inside the body — must also reject. */
    out_len = sizeof(out);
    compressed[compressed_len / 2] ^= (char)0xFF;
    rc = BZ2_bzBuffToBuffDecompress(
        out, &out_len, compressed, compressed_len,
        /* small */ 0, /* verbosity */ 0);
    assert(rc != BZ_OK);

    printf("bzip2 corruption rejection OK\n");
    return 0;
}
