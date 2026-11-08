/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for bzip2: file; calls BZ2_bzWriteOpen(), BZ2_bzWrite(), BZ2_bzWriteClose(), BZ2_bzReadOpen().
 */

#include <bzlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    static const char payload[] =
        "bzip2 FILE* API probe payload — repeated to be compressible. "
        "bzip2 FILE* API probe payload — repeated to be compressible. "
        "bzip2 FILE* API probe payload — repeated to be compressible. "
        "bzip2 FILE* API probe payload — repeated to be compressible.";
    const int payload_len = (int)sizeof(payload) - 1;

    char tmpl[] = "/tmp/demo_bzip2_XXXXXX";
    int fd = mkstemp(tmpl);
    assert(fd >= 0);
    close(fd);

    /* Write side. */
    FILE *fp = fopen(tmpl, "wb");
    assert(fp != NULL);
    int bzerr = BZ_OK;
    BZFILE *bzw = BZ2_bzWriteOpen(&bzerr, fp, /* blockSize100k */ 9, /* verbosity */ 0, /* workFactor */ 0);
    assert(bzerr == BZ_OK);
    assert(bzw != NULL);

    BZ2_bzWrite(&bzerr, bzw, (void *)payload, payload_len);
    assert(bzerr == BZ_OK);

    unsigned int bytes_in_lo = 0, bytes_in_hi = 0, bytes_out_lo = 0, bytes_out_hi = 0;
    BZ2_bzWriteClose(&bzerr, bzw, /* abandon */ 0, &bytes_in_lo, &bytes_out_lo);
    assert(bzerr == BZ_OK);
    assert(bytes_in_lo == (unsigned int)payload_len);
    assert(bytes_out_lo > 0);
    fclose(fp);

    /* Read side. */
    fp = fopen(tmpl, "rb");
    assert(fp != NULL);
    BZFILE *bzr = BZ2_bzReadOpen(&bzerr, fp, /* verbosity */ 0, /* small */ 0, NULL, 0);
    assert(bzerr == BZ_OK);
    assert(bzr != NULL);

    char roundtrip[4096];
    int total = 0;
    while (1) {
        int n = BZ2_bzRead(&bzerr, bzr, roundtrip + total, (int)sizeof(roundtrip) - total);
        if (bzerr == BZ_STREAM_END) {
            total += n;
            break;
        }
        assert(bzerr == BZ_OK);
        total += n;
        assert(total <= (int)sizeof(roundtrip));
    }
    assert(total == payload_len);
    assert(memcmp(roundtrip, payload, payload_len) == 0);

    BZ2_bzReadClose(&bzerr, bzr);
    assert(bzerr == BZ_OK);
    fclose(fp);

    unlink(tmpl);
    (void)bytes_in_hi;
    (void)bytes_out_hi;
    printf("bzip2 FILE* API OK: in=%u out=%u\n", bytes_in_lo, bytes_out_lo);
    return 0;
}
