/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freetype: stroker; calls FT_Init_FreeType(), FT_Stroker_New(), FT_Stroker_Set(), FT_Stroker_Rewind().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
#include <assert.h>
#include <stdio.h>

int main(void) {
    FT_Library library = NULL;
    assert(FT_Init_FreeType(&library) == 0);
    assert(library != NULL);

    FT_Stroker stroker = NULL;
    assert(FT_Stroker_New(library, &stroker) == 0);
    assert(stroker != NULL);

    FT_Stroker_Set(stroker,
                   64,                         /* radius (1px) */
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);                         /* miter_limit */

    /* Rewind resets the stroker to an empty state — safe to call on a fresh
     * stroker and exercises the internal border-reset code path. */
    FT_Stroker_Rewind(stroker);

    FT_Stroker_Set(stroker,
                   128,                        /* radius (2px) */
                   FT_STROKER_LINECAP_BUTT,
                   FT_STROKER_LINEJOIN_MITER_FIXED,
                   0x40000);                   /* miter_limit 4.0 in 16.16 */

    FT_Stroker_Done(stroker);
    assert(FT_Done_FreeType(library) == 0);

    printf("freetype stroker test OK\n");
    return 0;
}
