/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmagic: core API; calls magic_open(), magic_load(), magic_buffer(), magic_close().
 */

#include <magic.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    magic_t cookie = magic_open(MAGIC_NONE);
    assert(cookie != NULL);
    int rc = magic_load(cookie, NULL);
    assert(rc == 0);
    static const unsigned char png_header[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0,0,0,13,'I','H','D','R'
    };
    const char *desc = magic_buffer(cookie, png_header, sizeof(png_header));
    assert(desc != NULL);
    magic_close(cookie);
    printf("libmagic test OK\n");
    return 0;
}
