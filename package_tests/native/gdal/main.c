/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gdal: core API; calls GDALAllRegister(), GDALGetDriverCount(), GDALGetDriverByName(), OGRRegisterAll().
 */

#include <gdal.h>
#include <ogr_api.h>
#include <cpl_conv.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    GDALAllRegister();

    int raster_count = GDALGetDriverCount();
    assert(raster_count > 0);

    GDALDriverH gtiff = GDALGetDriverByName("GTiff");
    assert(gtiff != NULL);

    OGRRegisterAll();
    int vector_count = OGRGetDriverCount();
    assert(vector_count > 0);

    const char *version = GDALVersionInfo("RELEASE_NAME");
    assert(version != NULL);
    assert(strlen(version) > 0);

    /* GDALVersionInfo() returns a buffer owned by GDAL that the driver-manager
     * teardown frees, so print before tearing down. */
    printf("gdal test OK: %s (raster_drivers=%d, vector_drivers=%d)\n",
           version, raster_count, vector_count);

    GDALDestroyDriverManager();
    return 0;
}
