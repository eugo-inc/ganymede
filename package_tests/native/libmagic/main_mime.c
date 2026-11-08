/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmagic: mime; calls magic_open(), magic_load(), magic_buffer(), magic_close().
 */

#include <magic.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    magic_t cookie = magic_open(MAGIC_MIME_TYPE);
    assert(cookie != NULL);
    int rc = magic_load(cookie, NULL);
    assert(rc == 0);

    /* Minimal valid PNG header (signature + IHDR chunk start). */
    static const unsigned char png_header[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0,0,0,13,'I','H','D','R'
    };
    const char *mime = magic_buffer(cookie, png_header, sizeof(png_header));
    assert(mime != NULL);
    assert(strcmp(mime, "image/png") == 0);

    static const unsigned char gif_header[] = {
        'G','I','F','8','7','a', 1,0, 1,0, 0,0,0
    };
    const char *gif_mime = magic_buffer(cookie, gif_header, sizeof(gif_header));
    assert(gif_mime != NULL);
    assert(strcmp(gif_mime, "image/gif") == 0);

    magic_close(cookie);
    printf("libmagic MIME test OK\n");
    return 0;
}
