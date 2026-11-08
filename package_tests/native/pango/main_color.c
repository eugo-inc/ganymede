/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pango: color; calls pango_color_parse(), pango_color_parse_with_alpha(), pango_color_to_string(), g_free().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    PangoColor color;
    memset(&color, 0, sizeof(color));

    /* Hex form: #rrggbb. 0xff -> 0xffff in pango's 16-bit-per-channel model. */
    gboolean ok = pango_color_parse(&color, "#ff0000");
    assert(ok);
    assert(color.red == 0xffff);
    assert(color.green == 0x0000);
    assert(color.blue == 0x0000);

    /* Named color from the X11 database. */
    memset(&color, 0, sizeof(color));
    ok = pango_color_parse(&color, "white");
    assert(ok);
    assert(color.red == 0xffff);
    assert(color.green == 0xffff);
    assert(color.blue == 0xffff);

    /* Hex form #rgb shorthand: each nibble expands to a full byte. */
    memset(&color, 0, sizeof(color));
    ok = pango_color_parse(&color, "#0f0");
    assert(ok);
    assert(color.red == 0x0000);
    assert(color.green == 0xffff);
    assert(color.blue == 0x0000);

    /* Bad input must fail cleanly without crashing. */
    memset(&color, 0, sizeof(color));
    ok = pango_color_parse(&color, "not-a-color");
    assert(!ok);

    /* With-alpha variant: #rrggbbaa. */
    memset(&color, 0, sizeof(color));
    guint16 alpha = 0;
    ok = pango_color_parse_with_alpha(&color, &alpha, "#0000ff");
    assert(ok);
    assert(color.blue == 0xffff);
    assert(alpha == 0xffff);  /* No alpha given -> opaque. */

    /* Serialize back to hex. */
    PangoColor red = {0xffff, 0x0000, 0x0000};
    char *s = pango_color_to_string(&red);
    assert(s != NULL);
    /* Format is #RRRRGGGGBBBB (16 bits per channel, hex). */
    assert(s[0] == '#');
    assert(strlen(s) >= 7);
    g_free(s);

    /* Copy + free. */
    PangoColor src = {0x1111, 0x2222, 0x3333};
    PangoColor *dup = pango_color_copy(&src);
    assert(dup != NULL);
    assert(dup->red == 0x1111);
    assert(dup->green == 0x2222);
    assert(dup->blue == 0x3333);
    pango_color_free(dup);

    printf("pango color test OK\n");
    return 0;
}
