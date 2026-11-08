/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgeotiff: print; calls GTIFKeyName(), GTIFValueName(), GTIFTypeName(), mkstemps().
 */

#include <geotiff.h>
#include <geotiffio.h>
#include <geokeys.h>
#include <geovalues.h>
#include <geonames.h>
#include <xtiffio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* GTIFPrint dispatches lines through this signature. */
static int collect(char *line, void *aux) {
    char *buf = (char *)aux;
    /* GTIFPrint emits one line per call; concatenate into the buffer. */
    size_t cur = strlen(buf);
    size_t add = strlen(line);
    if (cur + add + 1 < 8192) {
        memcpy(buf + cur, line, add);
        buf[cur + add] = '\0';
    }
    return 0;
}

int main(void) {
    /* --- Name-lookup API (does NOT require a GTIF or TIFF handle). --- */

    /* GTIFKeyName maps a geokey_t → its textual identifier. */
    const char *kname = GTIFKeyName(GTModelTypeGeoKey);
    assert(kname != NULL);
    assert(strstr(kname, "GTModelType") != NULL);

    /* GTIFValueName maps (key, value) → the symbolic value name. */
    const char *vname = GTIFValueName(GTModelTypeGeoKey, ModelTypeGeographic);
    assert(vname != NULL);
    assert(strstr(vname, "Geographic") != NULL);

    /* GTIFTypeName maps a tagtype_t → its name (e.g. "Short"). */
    const char *tname = GTIFTypeName(TYPE_SHORT);
    assert(tname != NULL);
    assert(strlen(tname) > 0);

    /* --- GTIFPrint callback path: write a GeoTIFF, then dump via GTIFPrint. --- */

    char path[] = "/tmp/demo_libgeotiff_print_testXXXXXX.tif";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);

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
    assert(GTIFKeySet(gtif, GeographicTypeGeoKey, TYPE_SHORT, 1, 4326) == 1);
    assert(GTIFWriteKeys(gtif) == 1);
    GTIFFree(gtif);
    XTIFFClose(tif);

    tif = XTIFFOpen(path, "r");
    assert(tif != NULL);
    gtif = GTIFNew(tif);
    assert(gtif != NULL);

    char buf[8192];
    buf[0] = '\0';
    GTIFPrint(gtif, collect, buf);

    assert(strlen(buf) > 0);
    assert(strstr(buf, "GTModelType") != NULL);
    assert(strstr(buf, "GeographicType") != NULL);

    GTIFFree(gtif);
    XTIFFClose(tif);
    unlink(path);

    printf("libgeotiff test OK: introspection API "
           "(KeyName='%s' ValueName='%s' TypeName='%s' Print=%zu bytes)\n",
           kname, vname, tname, strlen(buf));
    return 0;
}
