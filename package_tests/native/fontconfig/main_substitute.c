/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fontconfig: substitute; calls FcInitLoadConfig(), FcPatternCreate(), FcPatternAddString(), FcPatternGetInteger().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Load just the configuration (no font cache scan, like main.c). */
    FcConfig *config = FcInitLoadConfig();
    assert(config != NULL);

    FcPattern *pat = FcPatternCreate();
    assert(pat != NULL);
    assert(FcPatternAddString(pat, FC_FAMILY, (const FcChar8 *)"Sans") == FcTrue);

    /* Before defaults: weight is unset → FcPatternGetInteger returns NoMatch. */
    int weight_before = 0;
    FcResult r_before = FcPatternGetInteger(pat, FC_WEIGHT, 0, &weight_before);
    assert(r_before == FcResultNoMatch);

    /* Apply pattern-class substitutions from the loaded config rules. Must succeed. */
    FcBool ok_sub = FcConfigSubstitute(config, pat, FcMatchPattern);
    assert(ok_sub == FcTrue);

    /* Fill in defaults for any properties still unset. This is idempotent
     * and never fails — but it MUST cause previously-unset properties like
     * FC_WEIGHT to acquire a value. */
    FcDefaultSubstitute(pat);

    /* After defaults: weight MUST be set, and to a value within the documented
     * weight range (thin..extrablack = 0..215 in fontconfig's enum). */
    int weight_after = 0;
    FcResult r_after = FcPatternGetInteger(pat, FC_WEIGHT, 0, &weight_after);
    assert(r_after == FcResultMatch);
    assert(weight_after >= FC_WEIGHT_THIN);
    assert(weight_after <= FC_WEIGHT_EXTRABLACK);

    /* Slant is also a defaulted property — must be set after FcDefaultSubstitute. */
    int slant_after = 0;
    FcResult r_slant = FcPatternGetInteger(pat, FC_SLANT, 0, &slant_after);
    assert(r_slant == FcResultMatch);

    /* DPI defaults to 75.0 unless overridden — just verify it acquired *some* value. */
    double dpi_after = 0.0;
    FcResult r_dpi = FcPatternGetDouble(pat, FC_DPI, 0, &dpi_after);
    assert(r_dpi == FcResultMatch);
    assert(dpi_after > 0.0);

    /* Family must have been preserved through both substitutions. */
    FcChar8 *family = NULL;
    FcResult r_fam = FcPatternGetString(pat, FC_FAMILY, 0, &family);
    assert(r_fam == FcResultMatch);
    assert(family != NULL);

    FcPatternDestroy(pat);
    FcConfigDestroy(config);
    FcFini();

    printf("fontconfig substitute test OK: weight=%d slant=%d dpi=%.1f\n",
           weight_after, slant_after, dpi_after);
    return 0;
}
