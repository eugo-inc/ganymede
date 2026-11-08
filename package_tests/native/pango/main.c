/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pango: core API; calls pango_version(), pango_version_string(), pango_font_description_from_string(), pango_font_description_get_family().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(pango_version() >= 10000);

    const char *vs = pango_version_string();
    assert(vs != NULL);
    assert(strlen(vs) > 0);

    PangoFontDescription *desc = pango_font_description_from_string("Sans Bold 12");
    assert(desc != NULL);

    const char *family = pango_font_description_get_family(desc);
    assert(family != NULL);
    assert(strcmp(family, "Sans") == 0);

    /* Size is stored in Pango units (1024 per point); 12pt -> 12 * PANGO_SCALE. */
    assert(pango_font_description_get_size(desc) == 12 * PANGO_SCALE);
    assert(pango_font_description_get_weight(desc) == PANGO_WEIGHT_BOLD);

    char *roundtrip = pango_font_description_to_string(desc);
    assert(roundtrip != NULL);
    assert(strstr(roundtrip, "Sans") != NULL);
    assert(strstr(roundtrip, "Bold") != NULL);
    g_free(roundtrip);

    pango_font_description_free(desc);

    /* PangoLanguage: canonical-string lookup must succeed for a well-known tag. */
    PangoLanguage *lang = pango_language_from_string("en-US");
    assert(lang != NULL);
    const char *lang_str = pango_language_to_string(lang);
    assert(lang_str != NULL);
    assert(strlen(lang_str) > 0);

    printf("pango test OK: %s\n", vs);
    return 0;
}
