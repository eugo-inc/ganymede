/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liblzf: errors; calls lzf_compress(), lzf_decompress().
 */

#include <lzf.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char input[] =
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog. "
        "the quick brown fox jumps over the lazy dog.";
    const unsigned int input_len = (unsigned int)(sizeof(input) - 1);

    /* ---- Aspect 1: output buffer too small at compress time → returns 0. ---- */
    unsigned char tiny_out[4]; /* Deliberately undersized. */
    unsigned int compress_rc =
        lzf_compress(input, input_len, tiny_out, (unsigned int)sizeof(tiny_out));
    assert(compress_rc == 0);

    /* ---- Aspect 2a: corrupt compressed input → decompress returns 0, errno=EINVAL. ---- */
    const unsigned char corrupt[] = {0x20, 0x00};
    unsigned char out_buf[256];
    errno = 0;
    unsigned int decompress_rc = lzf_decompress(
        corrupt, (unsigned int)sizeof(corrupt), out_buf, (unsigned int)sizeof(out_buf));
    assert(decompress_rc == 0);
    assert(errno == EINVAL);

    /* ---- Aspect 2b: output buffer too small at decompress time → returns 0, errno=E2BIG. ---- */
    /* Produce a legitimate compressed stream, then feed decompress a buffer
     * smaller than the original payload. */
    unsigned char compressed[512];
    unsigned int compressed_len =
        lzf_compress(input, input_len, compressed, (unsigned int)sizeof(compressed));
    assert(compressed_len != 0);

    unsigned char tiny_decompress_out[4];
    errno = 0;
    unsigned int decompress_too_small_rc = lzf_decompress(
        compressed, compressed_len, tiny_decompress_out, (unsigned int)sizeof(tiny_decompress_out));
    assert(decompress_too_small_rc == 0);
    assert(errno == E2BIG);

    printf("liblzf errors test OK\n");
    return 0;
}
