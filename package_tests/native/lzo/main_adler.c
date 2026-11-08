/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lzo: adler; calls lzo_adler32(), lzo_version(), lzo_version_string(), lzo_version_date().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <lzo/lzoconf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(lzo_init() == LZO_E_OK);

    /* Version interface — runtime values must be present and sensible. */
    assert(lzo_version() != 0);
    const char *vs = lzo_version_string();
    const char *vd = lzo_version_date();
    assert(vs != NULL && vs[0] != '\0');
    assert(vd != NULL && vd[0] != '\0');

    /* Adler-32 of the empty buffer is the documented seed 1. */
    lzo_uint32_t a_empty = lzo_adler32(0, NULL, 0);
    assert(a_empty == 1);

    /* Adler-32 of "abc" is a well-known constant (0x024d0127). */
    const unsigned char abc[3] = { 'a', 'b', 'c' };
    lzo_uint32_t a_abc = lzo_adler32(1, abc, 3);
    assert(a_abc == 0x024d0127UL);

    const unsigned char buf[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    lzo_uint32_t one_shot = lzo_adler32(1, buf, 8);
    lzo_uint32_t streamed = lzo_adler32(1, buf, 4);
    streamed = lzo_adler32(streamed, buf + 4, 4);
    assert(streamed == one_shot);

    printf("lzo adler/version test OK: v=0x%lx '%s' (%s)\n",
           (unsigned long)lzo_version(), vs, vd);
    return 0;
}
