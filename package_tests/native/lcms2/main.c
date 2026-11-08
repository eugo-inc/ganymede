/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lcms2: core API; calls cmsGetEncodedCMMversion(), cmsCreate_sRGBProfile(), cmsCreateLab4Profile(), cmsCreateTransform().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <lcms2.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void) {
    /* Encoded CMM version must be at least 2.0 (reported as 2000). */
    int version = cmsGetEncodedCMMversion();
    assert(version >= 2000);

    cmsHPROFILE hSRGB = cmsCreate_sRGBProfile();
    assert(hSRGB != NULL);

    cmsHPROFILE hLab = cmsCreateLab4Profile(NULL);
    assert(hLab != NULL);

    /* sRGB (8-bit RGB) -> Lab (double-precision CIE Lab) transform. */
    cmsHTRANSFORM xform = cmsCreateTransform(
        hSRGB, TYPE_RGB_8,
        hLab, TYPE_Lab_DBL,
        INTENT_RELATIVE_COLORIMETRIC, 0);
    assert(xform != NULL);

    /* Pure-white sRGB pixel -> CIE Lab. Expected: L* ~= 100, a* ~= 0, b* ~= 0. */
    unsigned char rgb_white[3] = {255, 255, 255};
    cmsCIELab lab = {0.0, 0.0, 0.0};
    cmsDoTransform(xform, rgb_white, &lab, 1);
    assert(fabs(lab.L - 100.0) < 0.5);
    assert(fabs(lab.a) < 0.5);
    assert(fabs(lab.b) < 0.5);

    cmsDeleteTransform(xform);
    cmsCloseProfile(hLab);
    cmsCloseProfile(hSRGB);

    printf("lcms2 test OK: encoded CMM version %d\n", version);
    return 0;
}
