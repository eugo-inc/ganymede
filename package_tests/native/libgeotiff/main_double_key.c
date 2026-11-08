/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgeotiff: double key; calls mkstemps(), XTIFFOpen(), GTIFNew(), TIFFSetField().
 */

#include <geotiff.h>
#include <geotiffio.h>
#include <geokeys.h>
#include <xtiffio.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libgeotiff_dbl_testXXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

    const double expected = 0.0174532925199433;  /* degree → radian */

    /* Write phase. */
    TIFF *tif = XTIFFOpen(path, "w");
    assert(tif != NULL);
    GTIF *gtif = GTIFNew(tif);
    assert(gtif != NULL);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, 1);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, 1);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    unsigned char pixel = 0;
    assert(TIFFWriteScanline(tif, &pixel, 0, 0) >= 0);

    /* TYPE_DOUBLE storage path — different from main.c's TYPE_SHORT path. */
    assert(GTIFKeySet(gtif, GeogAngularUnitSizeGeoKey, TYPE_DOUBLE, 1, expected) == 1);
    assert(GTIFWriteKeys(gtif) == 1);
    GTIFFree(gtif);
    XTIFFClose(tif);

    /* Read-back phase. */
    tif = XTIFFOpen(path, "r");
    assert(tif != NULL);
    gtif = GTIFNew(tif);
    assert(gtif != NULL);

    double got = 0.0;
    int n = GTIFKeyGet(gtif, GeogAngularUnitSizeGeoKey, &got, 0, 1);
    assert(n == 1);
    /* Exact byte equality: libgeotiff must not coerce or quantize doubles. */
    assert(got == expected);

    int size = 0;
    tagtype_t ktype = TYPE_UNKNOWN;
    int count = GTIFKeyInfo(gtif, GeogAngularUnitSizeGeoKey, &size, &ktype);
    assert(count == 1);
    assert(ktype == TYPE_DOUBLE);
    assert(size == (int)sizeof(double));

    GTIFFree(gtif);
    XTIFFClose(tif);
    unlink(path);

    printf("libgeotiff test OK: TYPE_DOUBLE GeoKey round-trip (%.16g rad/deg)\n", got);
    return 0;
}
