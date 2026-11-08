/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freetype: core API; calls FT_Init_FreeType(), FT_Library_Version(), FT_Done_FreeType().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <ft2build.h>
#include FT_FREETYPE_H
#include <assert.h>
#include <stdio.h>

int main(void) {
    FT_Library library = NULL;
    assert(FT_Init_FreeType(&library) == 0);
    assert(library != NULL);

    FT_Int major = 0, minor = 0, patch = 0;
    FT_Library_Version(library, &major, &minor, &patch);
    assert(major >= 2);

    assert(FT_Done_FreeType(library) == 0);

    printf("freetype test OK: %d.%d.%d\n", (int)major, (int)minor, (int)patch);
    return 0;
}
