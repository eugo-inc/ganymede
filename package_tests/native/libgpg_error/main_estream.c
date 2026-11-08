/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgpg_error: estream; calls gpgrt_fopenmem(), gpgrt_fwrite(), gpgrt_ftell(), gpgrt_fseek().
 */

#include <gpg-error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char payload[] =
        "Demo libgpg_error estream probe \x00 with embedded NUL and high bytes \xff\xfe";
    const size_t payload_len = sizeof(payload) - 1; /* keep trailing NUL OUT */

    /* Open an in-memory stream with no size limit ('+' = read-and-write). */
    gpgrt_stream_t s = gpgrt_fopenmem(0, "w+b");
    assert(s != NULL);

    /* Write the payload. fwrite returns nitems written, not byte count. */
    size_t nwritten = gpgrt_fwrite(payload, 1, payload_len, s);
    assert(nwritten == payload_len);

    /* Cursor should now be at end-of-stream. */
    long pos_after_write = gpgrt_ftell(s);
    assert(pos_after_write == (long)payload_len);

    /* Rewind. */
    int rc = gpgrt_fseek(s, 0, SEEK_SET);
    assert(rc == 0);
    assert(gpgrt_ftell(s) == 0);

    /* Read it back. */
    unsigned char readback[sizeof(payload)] = {0};
    size_t nread = gpgrt_fread(readback, 1, payload_len, s);
    assert(nread == payload_len);

    /* Byte-for-byte equality (memcmp, NOT strcmp — payload has embedded NUL). */
    assert(memcmp(readback, payload, payload_len) == 0);

    /* Cursor should be at end again. */
    assert(gpgrt_ftell(s) == (long)payload_len);

    rc = gpgrt_fclose(s);
    assert(rc == 0);

    printf("libgpg_error estream round-trip test OK: %zu bytes\n", payload_len);
    return 0;
}
