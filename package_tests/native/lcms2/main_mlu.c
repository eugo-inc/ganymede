/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lcms2: mlu; calls cmsMLUalloc(), cmsMLUsetASCII(), cmsMLUgetASCII(), cmsMLUtranslationsCount().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <lcms2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    cmsMLU *mlu = cmsMLUalloc(NULL, 1);
    assert(mlu != NULL);

    const char *english = "Demo lcms2 test profile";
    cmsBool ok = cmsMLUsetASCII(mlu, "en", "US", english);
    assert(ok);

    /* Two-pass read: 1st call returns the required buffer size, 2nd fills it. */
    cmsUInt32Number need = cmsMLUgetASCII(mlu, "en", "US", NULL, 0);
    assert(need == strlen(english) + 1);

    char buf[128] = {0};
    cmsUInt32Number got = cmsMLUgetASCII(mlu, "en", "US", buf, sizeof(buf));
    assert(got == need);
    assert(strcmp(buf, english) == 0);

    cmsUInt32Number n_trans = cmsMLUtranslationsCount(mlu);
    assert(n_trans == 1);

    /* Inspect the stored language/country triple at index 0. */
    char lang[3] = {0};
    char cntry[3] = {0};
    ok = cmsMLUtranslationsCodes(mlu, 0, lang, cntry);
    assert(ok);
    assert(strcmp(lang, "en") == 0);
    assert(strcmp(cntry, "US") == 0);

    cmsMLUfree(mlu);

    printf("lcms2 MLU test OK: round-trip \"%s\"\n", buf);
    return 0;
}
