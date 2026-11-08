/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liblzw: reject; calls lzw_open().
 */

#include <sys/types.h>
#include <lzw.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    assert(LZW_MAGIC_1 == 0x1F);
    assert(LZW_MAGIC_2 == 0x9D);

    /* Write a clearly-not-LZW payload (plain ASCII, no magic prefix). */
    char path[] = "/tmp/demo_liblzw_reject_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    const char garbage[] = "this is not a compressed stream\n";
    ssize_t written = write(fd, garbage, sizeof(garbage) - 1);
    assert(written == (ssize_t)(sizeof(garbage) - 1));
    close(fd);

    lzwFile *lzw = lzw_open(path, O_RDONLY);
    assert(lzw == NULL);  /* MUST reject — no LZW magic prefix. */

    unlink(path);

    lzwFile *missing = lzw_open("/tmp/demo_liblzw_does_not_exist_xyz", O_RDONLY);
    assert(missing == NULL);

    printf("liblzw reject test OK\n");
    return 0;
}
