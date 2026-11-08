/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for fontconfig: langset; calls FcLangSetCreate(), FcLangSetAdd(), FcLangSetHasLang(), FcLangSetCopy().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    FcLangSet *ls = FcLangSetCreate();
    assert(ls != NULL);

    assert(FcLangSetAdd(ls, (const FcChar8 *)"en") == FcTrue);
    assert(FcLangSetAdd(ls, (const FcChar8 *)"de") == FcTrue);

    /* Membership: exact-match languages must be present; an unrelated one absent. */
    FcLangResult r_en = FcLangSetHasLang(ls, (const FcChar8 *)"en");
    FcLangResult r_de = FcLangSetHasLang(ls, (const FcChar8 *)"de");
    FcLangResult r_xx = FcLangSetHasLang(ls, (const FcChar8 *)"xx");
    assert(r_en == FcLangEqual);
    assert(r_de == FcLangEqual);
    assert(r_xx == FcLangDifferentLang);

    FcLangSet *ls_copy = FcLangSetCopy(ls);
    assert(ls_copy != NULL);
    FcLangResult cmp = FcLangSetCompare(ls, ls_copy);
    assert(cmp == FcLangEqual);
    assert(FcLangSetEqual(ls, ls_copy) == FcTrue);

    FcStrSet *langs = FcLangSetGetLangs(ls);
    assert(langs != NULL);
    FcStrList *lit = FcStrListCreate(langs);
    assert(lit != NULL);
    int saw_en = 0, saw_de = 0;
    FcChar8 *tag;
    while ((tag = FcStrListNext(lit)) != NULL) {
        if (strcmp((const char *)tag, "en") == 0) saw_en = 1;
        if (strcmp((const char *)tag, "de") == 0) saw_de = 1;
    }
    assert(saw_en == 1);
    assert(saw_de == 1);
    FcStrListDone(lit);
    FcStrSetDestroy(langs);

    FcStrSet *defaults = FcGetDefaultLangs();
    assert(defaults != NULL);
    FcStrList *it = FcStrListCreate(defaults);
    assert(it != NULL);
    int seen = 0;
    while (FcStrListNext(it) != NULL) {
        seen++;
        if (seen > 1) break;
    }
    assert(seen >= 1);
    FcStrListDone(it);
    FcStrSetDestroy(defaults);

    FcLangSetDestroy(ls_copy);
    FcLangSetDestroy(ls);
    FcFini();

    printf("fontconfig langset test OK: en/de present, enumerated back\n");
    return 0;
}
