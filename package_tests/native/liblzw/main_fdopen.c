/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for liblzw: fdopen; calls lzw_fdopen(), lzw_read(), lzw_close().
 */

#include <sys/types.h>
#include <lzw.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const unsigned char SIMPLE_Z[] = {
    0x1f, 0x9d, 0x90, 0x31, 0x64, 0xcc, 0xa0, 0x51, 0xc3, 0xc6, 0x0d, 0x1c,
    0x39, 0x60, 0x84, 0x11, 0x33, 0x86, 0x4c, 0x19, 0x33, 0x67, 0xd0, 0xa4,
    0x51, 0xb3, 0x86, 0x4d, 0x1b, 0x37, 0x6f, 0xe0, 0xc4, 0x91, 0x33, 0x87,
    0x4e, 0x1d, 0x3b, 0x77, 0xf0, 0xe4, 0xd1, 0x13, 0x70, 0x60, 0xc1, 0x83,
    0x09, 0x17, 0x36, 0x7c, 0x18, 0x71, 0x62, 0xc5, 0x8b, 0x19, 0x37, 0x76,
    0xfc, 0x18, 0x72, 0xa4, 0x02,
};

int main(void) {
    char path[] = "/tmp/demo_liblzw_fdopen_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    ssize_t written = write(fd, SIMPLE_Z, sizeof(SIMPLE_Z));
    assert(written == (ssize_t)sizeof(SIMPLE_Z));
    /* Rewind so lzw_fdopen reads from byte 0 — it inherits the current offset. */
    off_t rewound = lseek(fd, 0, SEEK_SET);
    assert(rewound == 0);

    lzwFile *lzw = lzw_fdopen(fd);
    assert(lzw != NULL);

    char buf[16] = {0};
    ssize_t n = lzw_read(lzw, buf, 10);
    assert(n == 10);
    assert(memcmp(buf, "1234567890", 10) == 0);

    assert(lzw_close(lzw) == 0);
    unlink(path);

    printf("liblzw fdopen test OK\n");
    return 0;
}
