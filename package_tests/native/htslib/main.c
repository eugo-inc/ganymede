/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for htslib: core API; calls hts_version(), bgzf_open(), bgzf_write(), bgzf_close().
 */

#include <htslib/hts.h>
#include <htslib/bgzf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    const char *version = hts_version();
    assert(version != NULL && version[0] != '\0');
    printf("htslib version: %s\n", version);

    static const char payload[] =
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog "
        "the quick brown fox jumps over the lazy dog";
    const int payload_len = (int)sizeof(payload) - 1;

    char path[] = "/tmp/demo_htslib_smoke_XXXXXX";
    const int fd = mkstemp(path);
    assert(fd >= 0);
    close(fd);

    BGZF *out = bgzf_open(path, "w");
    assert(out != NULL);
    assert(bgzf_write(out, payload, (size_t)payload_len) == payload_len);
    assert(bgzf_close(out) == 0);

    BGZF *in = bgzf_open(path, "r");
    assert(in != NULL);
    char read_back[512] = {0};
    const ssize_t got = bgzf_read(in, read_back, sizeof(read_back) - 1);
    assert(bgzf_close(in) == 0);
    unlink(path);

    assert(got == payload_len);
    assert(memcmp(read_back, payload, (size_t)payload_len) == 0);

    printf("bgzf round-trip OK (%d bytes)\n", payload_len);
    return 0;
}
