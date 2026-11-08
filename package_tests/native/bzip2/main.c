/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for bzip2: core API; calls BZ2_bzBuffToBuffCompress(), BZ2_bzBuffToBuffDecompress(), BZ2_bzlibVersion().
 */

#include <bzlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char payload[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const unsigned int payload_len = (unsigned int)sizeof(payload) - 1;

    char compressed[4096];
    unsigned int compressed_len = sizeof(compressed);
    int rc = BZ2_bzBuffToBuffCompress(
        compressed, &compressed_len,
        (char *)payload, payload_len,
        /* blockSize100k */ 9, /* verbosity */ 0, /* workFactor */ 0);
    assert(rc == BZ_OK);
    assert(compressed_len > 0);
    assert(compressed_len < sizeof(compressed));

    char roundtrip[4096];
    unsigned int roundtrip_len = sizeof(roundtrip);
    rc = BZ2_bzBuffToBuffDecompress(
        roundtrip, &roundtrip_len,
        compressed, compressed_len,
        /* small */ 0, /* verbosity */ 0);
    assert(rc == BZ_OK);
    assert(roundtrip_len == payload_len);
    assert(memcmp(roundtrip, payload, payload_len) == 0);

    const char *version = BZ2_bzlibVersion();
    assert(version != NULL);
    assert(version[0] != '\0');

    printf("bzip2 test OK: %s\n", version);
    return 0;
}
