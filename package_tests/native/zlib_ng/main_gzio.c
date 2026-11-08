/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zlib_ng: gzio; calls mkstemps(), zng_gzopen(), zng_gzwrite(), zng_gzread(), zng_gzeof(), zng_gzclose().
 */

/* zlib-ng.h declares the zng_gz* functions only when WITH_GZFILEOP is defined. zlib-ng.pc adds it to
 * Cflags; this probe is compiled without pkg-config, so it defines it itself. */
#ifndef WITH_GZFILEOP
#define WITH_GZFILEOP
#endif
#include <zlib-ng.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    const char *payload =
        "Demo zlib_ng gzip file I/O probe: exercises zng_gzopen/zng_gzwrite/zng_gzread/zng_gzclose.\n"
        "A second line to push us past trivial single-block sizes.\n";
    const int32_t payload_len = (int32_t)strlen(payload);

    char path[] = "/tmp/demo_zlib_ng_gzio_XXXXXX.gz";
    int fd = mkstemps(path, 3);
    assert(fd >= 0);
    close(fd);

    /* Write side. */
    gzFile gz_w = zng_gzopen(path, "wb");
    assert(gz_w != NULL);
    int32_t written = zng_gzwrite(gz_w, payload, (uint32_t)payload_len);
    assert(written == payload_len);
    int32_t rc = zng_gzclose(gz_w);
    assert(rc == Z_OK);

    /* Read side. */
    char buf[512] = {0};
    gzFile gz_r = zng_gzopen(path, "rb");
    assert(gz_r != NULL);
    int32_t read_n = zng_gzread(gz_r, buf, (uint32_t)(sizeof(buf) - 1));
    assert(read_n == payload_len);
    /* A second read should now hit EOF. */
    char trailer[16];
    read_n = zng_gzread(gz_r, trailer, (uint32_t)sizeof(trailer));
    assert(read_n == 0);
    assert(zng_gzeof(gz_r));
    rc = zng_gzclose(gz_r);
    assert(rc == Z_OK);

    assert(memcmp(buf, payload, (size_t)payload_len) == 0);

    unlink(path);
    printf("zlib_ng gzio probe OK: %d bytes round-tripped via %s\n", (int)payload_len, path);
    return 0;
}
