/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pango: script; calls pango_script_for_unichar(), pango_script_get_sample_language(), pango_language_to_string(), pango_script_iter_new().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Latin 'A' (U+0041). */
    assert(pango_script_for_unichar(0x0041) == PANGO_SCRIPT_LATIN);

    /* Greek alpha (U+03B1). */
    assert(pango_script_for_unichar(0x03B1) == PANGO_SCRIPT_GREEK);

    /* Cyrillic small letter a (U+0430). */
    assert(pango_script_for_unichar(0x0430) == PANGO_SCRIPT_CYRILLIC);

    /* Han: CJK unified ideograph "中" (U+4E2D). */
    assert(pango_script_for_unichar(0x4E2D) == PANGO_SCRIPT_HAN);

    /* Arabic letter alef (U+0627). */
    assert(pango_script_for_unichar(0x0627) == PANGO_SCRIPT_ARABIC);

    /* Hiragana 'あ' (U+3042). */
    assert(pango_script_for_unichar(0x3042) == PANGO_SCRIPT_HIRAGANA);

    /* Hebrew letter alef (U+05D0). */
    assert(pango_script_for_unichar(0x05D0) == PANGO_SCRIPT_HEBREW);

    /* ASCII digit '0' (U+0030) is COMMON, not LATIN. */
    assert(pango_script_for_unichar(0x0030) == PANGO_SCRIPT_COMMON);

    /* Space (U+0020) is COMMON. */
    assert(pango_script_for_unichar(0x0020) == PANGO_SCRIPT_COMMON);

    /* Sample-language lookup for Latin should return a non-NULL language
     * whose tag is a plausible BCP-47 string. */
    PangoLanguage *latin_lang = pango_script_get_sample_language(PANGO_SCRIPT_LATIN);
    /* sample_language CAN return NULL for some scripts (no representative
     * language), but Latin specifically should map to something. */
    assert(latin_lang != NULL);
    const char *latin_tag = pango_language_to_string(latin_lang);
    assert(latin_tag != NULL);
    assert(strlen(latin_tag) > 0);

    const char *mixed = "Hello \xce\xb1\xce\xb2";  /* "Hello αβ" (UTF-8) */
    PangoScriptIter *it = pango_script_iter_new(mixed, -1);
    assert(it != NULL);

    int saw_latin = 0;
    int saw_greek = 0;
    do {
        const char *start = NULL;
        const char *end = NULL;
        PangoScript script = PANGO_SCRIPT_INVALID_CODE;
        pango_script_iter_get_range(it, &start, &end, &script);
        assert(start != NULL);
        assert(end != NULL);
        assert(end > start);
        if (script == PANGO_SCRIPT_LATIN) saw_latin = 1;
        if (script == PANGO_SCRIPT_GREEK) saw_greek = 1;
    } while (pango_script_iter_next(it));

    assert(saw_latin);
    assert(saw_greek);
    pango_script_iter_free(it);

    printf("pango script test OK\n");
    return 0;
}
