/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freetype: bitmap; calls FT_Init_FreeType(), FT_Bitmap_Init(), FT_Bitmap_Done(), FT_Outline_New().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_OUTLINE_H
#include <assert.h>
#include <stdio.h>

int main(void) {
    FT_Library library = NULL;
    assert(FT_Init_FreeType(&library) == 0);

    /* --- Bitmap lifecycle --- */
    FT_Bitmap bm;
    FT_Bitmap_Init(&bm);
    /* Just-initialized bitmap is fully zeroed. */
    assert(bm.buffer == NULL);
    assert(bm.width == 0);
    assert(bm.rows == 0);
    assert(bm.pitch == 0);
    assert(bm.num_grays == 0);
    assert(FT_Bitmap_Done(library, &bm) == 0);

    /* --- Outline lifecycle --- */
    FT_Outline outline;
    /* Allocate an outline sized for 3 points / 1 contour — a triangle's worth.
     * FT_Outline_New allocates the points/contours/tags arrays and records the
     * requested sizes in n_points / n_contours (FreeType treats these as the
     * "size" of the outline, not a separate capacity vs. usage split). */
    assert(FT_Outline_New(library, 3, 1, &outline) == 0);
    assert(outline.points != NULL);
    assert(outline.contours != NULL);
    assert(outline.tags != NULL);
    assert(outline.n_points == 3);
    assert(outline.n_contours == 1);

    assert(FT_Outline_Done(library, &outline) == 0);

    assert(FT_Done_FreeType(library) == 0);
    printf("freetype bitmap+outline test OK\n");
    return 0;
}
