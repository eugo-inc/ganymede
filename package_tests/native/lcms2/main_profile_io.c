/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for lcms2: profile io; calls cmsCreate_sRGBProfile(), cmsSaveProfileToMem(), cmsOpenProfileFromMem(), cmsGetColorSpace().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <lcms2.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    cmsHPROFILE src = cmsCreate_sRGBProfile();
    assert(src != NULL);

    cmsUInt32Number need = 0;
    cmsBool ok = cmsSaveProfileToMem(src, NULL, &need);
    assert(ok);
    assert(need > 128);  /* ICC profile minimum header alone is 128 bytes. */

    unsigned char *buf = (unsigned char *)malloc(need);
    assert(buf != NULL);
    cmsUInt32Number written = need;
    ok = cmsSaveProfileToMem(src, buf, &written);
    assert(ok);
    assert(written == need);

    /* ICC v2/v4 profiles have the magic signature "acsp" at byte offset 36. */
    assert(memcmp(buf + 36, "acsp", 4) == 0);

    cmsHPROFILE reopened = cmsOpenProfileFromMem(buf, need);
    assert(reopened != NULL);

    cmsColorSpaceSignature src_cs = cmsGetColorSpace(src);
    cmsColorSpaceSignature dst_cs = cmsGetColorSpace(reopened);
    assert(src_cs == dst_cs);
    assert(src_cs == cmsSigRgbData);

    cmsProfileClassSignature klass = cmsGetDeviceClass(reopened);
    assert(klass == cmsSigDisplayClass);

    cmsCloseProfile(reopened);
    cmsCloseProfile(src);
    free(buf);

    printf("lcms2 profile-IO test OK: %u bytes round-tripped\n", need);
    return 0;
}
