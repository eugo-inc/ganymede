/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libunistring: core API; calls u8_check(), u8_strlen().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <unistring/version.h>
#include <unistr.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Version macro from <unistring/version.h> must be non-zero. */
    assert(_LIBUNISTRING_VERSION != 0);

    /* Valid UTF-8: "héllo" — 'h', U+00E9 (2 bytes), 'l','l','o'. u8_check returns NULL on success. */
    static const uint8_t valid_utf8[] = {0x68, 0xC3, 0xA9, 0x6C, 0x6C, 0x6F};
    assert(u8_check(valid_utf8, sizeof(valid_utf8)) == NULL);

    /* u8_strlen counts code units (bytes) up to NUL, not code points. */
    static const uint8_t nul_terminated[] = {0x68, 0xC3, 0xA9, 0x00};
    assert(u8_strlen(nul_terminated) == 3);

    /* Invalid UTF-8 (lone continuation byte 0x80) must be rejected. */
    static const uint8_t invalid_utf8[] = {0x80};
    assert(u8_check(invalid_utf8, sizeof(invalid_utf8)) != NULL);

    printf("libunistring test OK: version=0x%x\n", _LIBUNISTRING_VERSION);
    return 0;
}
