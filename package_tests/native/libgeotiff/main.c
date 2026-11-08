/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgeotiff: core API; calls mkstemps(), XTIFFOpen(), GTIFNew(), TIFFSetField().
 */

#include <geotiff.h>
#include <geotiffio.h>
#include <xtiffio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libgeotiff_testXXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

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

    assert(GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeGeographic) == 1);
    assert(GTIFWriteKeys(gtif) == 1);
    GTIFFree(gtif);
    XTIFFClose(tif);

    /* Read-back phase. */
    tif = XTIFFOpen(path, "r");
    assert(tif != NULL);
    gtif = GTIFNew(tif);
    assert(gtif != NULL);

    short model = 0;
    int got = GTIFKeyGet(gtif, GTModelTypeGeoKey, &model, 0, 1);
    assert(got == 1);
    assert(model == ModelTypeGeographic);

    GTIFFree(gtif);
    XTIFFClose(tif);
    unlink(path);

    printf("libgeotiff test OK: GeoKey round-trip verified (ModelTypeGeographic)\n");
    return 0;
}
