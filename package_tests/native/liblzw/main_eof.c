/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liblzw: eof; calls lzw_open(), lzw_read(), lzw_close().
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
    char path[] = "/tmp/demo_liblzw_eof_XXXXXX";
    int fd = mkstemp(path);
    assert(fd >= 0);
    ssize_t written = write(fd, SIMPLE_Z, sizeof(SIMPLE_Z));
    assert(written == (ssize_t)sizeof(SIMPLE_Z));
    close(fd);

    lzwFile *lzw = lzw_open(path, O_RDONLY);
    assert(lzw != NULL);

    unsigned char buf[256] = {0};
    size_t total = 0;
    while (total < sizeof(buf)) {
        ssize_t n = lzw_read(lzw, buf + total, sizeof(buf) - total);
        assert(n >= 0);
        if (n == 0) {
            break;  /* EOF */
        }
        total += (size_t)n;
    }
    assert(total == 73);
    assert(memcmp(buf, "1234567890abcdefghijklmnopqrstuvwxyz"
                       "1234567890abcdefghijklmnopqrstuvwxyz\n", 73) == 0);

    /* Confirm a read past EOF returns exactly 0 (not -1, not blocking). */
    unsigned char dummy[4];
    ssize_t past = lzw_read(lzw, dummy, sizeof(dummy));
    assert(past == 0);

    assert(lzw_close(lzw) == 0);
    unlink(path);

    printf("liblzw eof test OK (%zu bytes decompressed)\n", total);
    return 0;
}
