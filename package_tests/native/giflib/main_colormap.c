/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for giflib: colormap; calls GifBitSize(), GifMakeMapObject(), GifUnionColorMap(), GifFreeMapObject().
 */

#include <gif_lib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* GifBitSize: returns the smallest n such that (1 << n) >= count, for
     * count in [2, 256]. Documented behavior — verify a few anchor points. */
    assert(GifBitSize(2) == 1);
    assert(GifBitSize(3) == 2);
    assert(GifBitSize(4) == 2);
    assert(GifBitSize(5) == 3);
    assert(GifBitSize(16) == 4);
    assert(GifBitSize(17) == 5);
    assert(GifBitSize(256) == 8);

    GifColorType a_colors[2] = { {0, 0, 0}, {255, 255, 255} };
    GifColorType b_colors[2] = { {255, 0, 0}, {0, 255, 0} };  /* disjoint from a */

    ColorMapObject *a = GifMakeMapObject(2, a_colors);
    ColorMapObject *b = GifMakeMapObject(2, b_colors);
    assert(a != NULL);
    assert(b != NULL);
    assert(a->ColorCount == 2);
    assert(b->ColorCount == 2);
    assert(a->BitsPerPixel == 1);
    assert(b->BitsPerPixel == 1);
    /* GifMakeMapObject should copy the colors in. */
    assert(a->Colors[0].Red == 0   && a->Colors[0].Green == 0   && a->Colors[0].Blue == 0);
    assert(a->Colors[1].Red == 255 && a->Colors[1].Green == 255 && a->Colors[1].Blue == 255);

    GifPixelType translation[256] = {0};
    ColorMapObject *u = GifUnionColorMap(a, b, translation);
    assert(u != NULL);
    assert(u->ColorCount == 4);
    /* Sized up to fit 4 entries → bits-per-pixel = 2 (since 1<<2 == 4). */
    assert(u->BitsPerPixel == 2);
    /* a's entries occupy positions 0,1 (unchanged); b's entries appended
     * to the end of the merged palette. */
    assert(u->Colors[0].Red == 0   && u->Colors[0].Blue == 0);
    assert(u->Colors[1].Red == 255 && u->Colors[1].Green == 255);
    /* Translation table must remap every index of b into the merged map. */
    assert(translation[0] < u->ColorCount);
    assert(translation[1] < u->ColorCount);
    /* And the colors those translated indices point at must equal b's
     * original colors — that's what the translation table is FOR. */
    assert(u->Colors[translation[0]].Red == b_colors[0].Red);
    assert(u->Colors[translation[0]].Green == b_colors[0].Green);
    assert(u->Colors[translation[0]].Blue == b_colors[0].Blue);
    assert(u->Colors[translation[1]].Red == b_colors[1].Red);
    assert(u->Colors[translation[1]].Green == b_colors[1].Green);
    assert(u->Colors[translation[1]].Blue == b_colors[1].Blue);

    GifFreeMapObject(a);
    GifFreeMapObject(b);
    GifFreeMapObject(u);

    printf("giflib colormap test OK\n");
    return 0;
}
