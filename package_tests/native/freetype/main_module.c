/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freetype: module; calls FT_Init_FreeType(), FT_Get_Module(), FT_Done_FreeType().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
#include <assert.h>
#include <stdio.h>

int main(void) {
    FT_Library library = NULL;
    assert(FT_Init_FreeType(&library) == 0);

    const char* required[] = {"sfnt", "truetype", "type1", "cff"};
    const size_t n = sizeof(required) / sizeof(required[0]);

    for (size_t i = 0; i < n; ++i) {
        FT_Module mod = FT_Get_Module(library, required[i]);
        if (mod == NULL) {
            fprintf(stderr, "freetype module missing: %s\n", required[i]);
            return 1;
        }
    }

    /* A name that is guaranteed NOT to be a freetype module should return NULL —
     * proves FT_Get_Module is actually checking the registry, not stubbed. */
    assert(FT_Get_Module(library, "demo_definitely_not_a_real_freetype_module") == NULL);

    assert(FT_Done_FreeType(library) == 0);
    printf("freetype module test OK: %zu drivers present\n", n);
    return 0;
}
