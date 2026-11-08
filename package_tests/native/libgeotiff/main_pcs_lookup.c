/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgeotiff: pcs lookup; calls mkstemps(), XTIFFOpen(), GTIFNew(), TIFFSetField().
 */

#include <geotiff.h>
#include <geotiffio.h>
#include <geokeys.h>
#include <geovalues.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libgeotiff_pcs_testXXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

    const unsigned short epsg_utm11n = 32611;

    /* Write phase: stamp a Projected model + EPSG code into a minimal TIFF. */
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

    assert(GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected) == 1);
    assert(GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, epsg_utm11n) == 1);
    assert(GTIFWriteKeys(gtif) == 1);
    GTIFFree(gtif);
    XTIFFClose(tif);

    /* Read-back + EPSG expansion phase. */
    tif = XTIFFOpen(path, "r");
    assert(tif != NULL);
    gtif = GTIFNew(tif);
    assert(gtif != NULL);

    GTIFDefn defn;
    memset(&defn, 0, sizeof(defn));
    int ok = GTIFGetDefn(gtif, &defn);
    assert(ok == 1);

    assert(defn.PCS == epsg_utm11n);

    /* Model must classify as Projected, not Geographic. */
    assert(defn.Model == ModelTypeProjected);

    /* UTM zone 11N → projection family is Transverse Mercator (CT_TransverseMercator = 1)
     * per the GeoTIFF coord-transform code registry. A failure here means PROJ either
     * didn't resolve the EPSG code at all or returned the wrong projection family. */
    assert(defn.CTProjection == CT_TransverseMercator);

    assert(defn.SemiMajor > 6.0e6);
    assert(defn.SemiMajor < 6.5e6);

    GTIFFree(gtif);
    XTIFFClose(tif);
    unlink(path);

    printf("libgeotiff test OK: EPSG %u expanded via PROJ "
           "(Model=%d CTProjection=%d SemiMajor=%.3f m)\n",
           epsg_utm11n, defn.Model, defn.CTProjection, defn.SemiMajor);
    return 0;
}
