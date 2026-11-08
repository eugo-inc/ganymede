/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmagic: compress; calls magic_open(), magic_load(), magic_buffer(), magic_close().
 */

#include <magic.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* --- Aspect 1: gzip detection (proves compression magic db loaded). --- */
    magic_t cookie = magic_open(MAGIC_NONE);
    assert(cookie != NULL);
    int rc = magic_load(cookie, NULL);
    assert(rc == 0);

    /* Minimal valid gzip header: 1f 8b 08 (magic + deflate method) + flags
     * + mtime(4) + xfl + os. This is enough for libmagic to classify the
     * stream as "gzip compressed data". */
    static const unsigned char gzip_blob[] = {
        0x1F, 0x8B, 0x08, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x03
    };
    const char *desc = magic_buffer(cookie, gzip_blob, sizeof(gzip_blob));
    assert(desc != NULL);
    /* Case-insensitive substring "gzip" is the structural assertion. */
    int found_gzip = 0;
    for (const char *p = desc; *p; p++) {
        if ((p[0] == 'g' || p[0] == 'G') &&
            (p[1] == 'z' || p[1] == 'Z') &&
            (p[2] == 'i' || p[2] == 'I') &&
            (p[3] == 'p' || p[3] == 'P')) {
            found_gzip = 1;
            break;
        }
    }
    assert(found_gzip);

    /* --- Aspect 2: MAGIC_EXTENSION flag (distinct from MIME / NONE). --- */
    magic_t c2 = magic_open(MAGIC_EXTENSION);
    assert(c2 != NULL);
    int rc2 = magic_load(c2, NULL);
    assert(rc2 == 0);

    static const unsigned char png_header[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0,0,0,13,'I','H','D','R'
    };
    const char *ext = magic_buffer(c2, png_header, sizeof(png_header));
    assert(ext != NULL);
    assert(strstr(ext, "png") != NULL || strstr(ext, "PNG") != NULL);
    /* And it must NOT contain the verbose words from MAGIC_NONE output. */
    assert(strstr(ext, "image data") == NULL);

    magic_close(c2);
    magic_close(cookie);
    printf("libmagic compress/extension test OK\n");
    return 0;
}
