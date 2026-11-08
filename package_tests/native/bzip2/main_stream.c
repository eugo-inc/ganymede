/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for bzip2: stream; calls BZ2_bzCompressInit(), BZ2_bzCompress(), BZ2_bzCompressEnd(), BZ2_bzDecompressInit().
 */

#include <bzlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    enum { PAYLOAD_LEN = 16384 };
    char *payload = malloc(PAYLOAD_LEN);
    assert(payload != NULL);
    for (int i = 0; i < PAYLOAD_LEN; ++i) {
        payload[i] = (char)('A' + (i % 26));
    }

    char compressed[PAYLOAD_LEN + 1024];
    unsigned int total_compressed = 0;

    bz_stream cs;
    memset(&cs, 0, sizeof(cs));
    int rc = BZ2_bzCompressInit(&cs, /* blockSize100k */ 1, /* verbosity */ 0, /* workFactor */ 0);
    assert(rc == BZ_OK);

    cs.next_in = payload;
    cs.avail_in = PAYLOAD_LEN;
    cs.next_out = compressed;
    cs.avail_out = sizeof(compressed);

    /* Feed all input in a single BZ_RUN; bzip2 may not consume it all in one call. */
    while (cs.avail_in > 0) {
        rc = BZ2_bzCompress(&cs, BZ_RUN);
        assert(rc == BZ_RUN_OK);
    }
    /* Then finish — may need multiple iterations until BZ_STREAM_END. */
    do {
        rc = BZ2_bzCompress(&cs, BZ_FINISH);
        assert(rc == BZ_FINISH_OK || rc == BZ_STREAM_END);
    } while (rc != BZ_STREAM_END);
    total_compressed = (unsigned int)(sizeof(compressed) - cs.avail_out);
    assert(total_compressed > 0);
    assert(total_compressed < PAYLOAD_LEN);  /* must actually compress */

    rc = BZ2_bzCompressEnd(&cs);
    assert(rc == BZ_OK);

    /* Now decompress incrementally. */
    char *roundtrip = malloc(PAYLOAD_LEN + 128);
    assert(roundtrip != NULL);

    bz_stream ds;
    memset(&ds, 0, sizeof(ds));
    rc = BZ2_bzDecompressInit(&ds, /* verbosity */ 0, /* small */ 0);
    assert(rc == BZ_OK);

    ds.next_in = compressed;
    ds.avail_in = total_compressed;
    ds.next_out = roundtrip;
    ds.avail_out = PAYLOAD_LEN + 128;

    do {
        rc = BZ2_bzDecompress(&ds);
        assert(rc == BZ_OK || rc == BZ_STREAM_END);
    } while (rc != BZ_STREAM_END);
    unsigned int roundtrip_len = (unsigned int)((PAYLOAD_LEN + 128) - ds.avail_out);
    assert(roundtrip_len == PAYLOAD_LEN);
    assert(memcmp(roundtrip, payload, PAYLOAD_LEN) == 0);

    rc = BZ2_bzDecompressEnd(&ds);
    assert(rc == BZ_OK);

    free(payload);
    free(roundtrip);
    printf("bzip2 streaming API OK\n");
    return 0;
}
