/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fontconfig: name parse; calls FcPatternCreate(), FcPatternAddString(), FcPatternAddInteger(), FcPatternAddDouble().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    FcPattern *src = FcPatternCreate();
    assert(src != NULL);
    assert(FcPatternAddString(src, FC_FAMILY, (const FcChar8 *)"Helvetica") == FcTrue);
    assert(FcPatternAddInteger(src, FC_WEIGHT, FC_WEIGHT_BOLD) == FcTrue);
    assert(FcPatternAddInteger(src, FC_SLANT, FC_SLANT_ITALIC) == FcTrue);
    assert(FcPatternAddDouble(src, FC_SIZE, 12.0) == FcTrue);

    /* Unparse → canonical textual form. Must be a non-empty string mentioning
     * the family name (the most stable substring across fontconfig versions). */
    FcChar8 *unparsed = FcNameUnparse(src);
    assert(unparsed != NULL);
    assert(strlen((const char *)unparsed) > 0);
    assert(strstr((const char *)unparsed, "Helvetica") != NULL);

    /* Parse the canonical form back. The reparsed pattern must contain the
     * same family/weight/slant/size — verifying both directions of the grammar. */
    FcPattern *dst = FcNameParse(unparsed);
    assert(dst != NULL);

    FcChar8 *family = NULL;
    assert(FcPatternGetString(dst, FC_FAMILY, 0, &family) == FcResultMatch);
    assert(family != NULL);
    assert(strcmp((const char *)family, "Helvetica") == 0);

    int weight = 0;
    assert(FcPatternGetInteger(dst, FC_WEIGHT, 0, &weight) == FcResultMatch);
    assert(weight == FC_WEIGHT_BOLD);

    int slant = 0;
    assert(FcPatternGetInteger(dst, FC_SLANT, 0, &slant) == FcResultMatch);
    assert(slant == FC_SLANT_ITALIC);

    double size = 0.0;
    assert(FcPatternGetDouble(dst, FC_SIZE, 0, &size) == FcResultMatch);
    assert(size == 12.0);

    /* FcPatternEqual must agree the two patterns are equivalent. */
    assert(FcPatternEqual(src, dst) == FcTrue);

    FcPattern *garbage = FcNameParse((const FcChar8 *)"");
    if (garbage != NULL) {
        assert(FcPatternEqual(src, garbage) == FcFalse);
        FcPatternDestroy(garbage);
    }

    FcPatternDestroy(dst);
    FcStrFree(unparsed);
    FcPatternDestroy(src);

    printf("fontconfig name-parse test OK: pattern round-trip equal\n");
    return 0;
}
