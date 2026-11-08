/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for netcdf_c: core API; calls nc_inq_libvers(), mkstemps(), nc_create(), nc_def_dim().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <netcdf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    /* Version string must be non-empty. */
    const char *v = nc_inq_libvers();
    assert(v != NULL);
    assert(strlen(v) > 0);

    char path[] = "/tmp/demo_netcdf_test_XXXXXX.nc";
    int fd = mkstemps(path, 3);
    assert(fd >= 0);
    close(fd);

    int ncid = -1;
    assert(nc_create(path, NC_CLOBBER, &ncid) == NC_NOERR);

    int dimid = -1;
    assert(nc_def_dim(ncid, "n", 4, &dimid) == NC_NOERR);

    int varid = -1;
    assert(nc_def_var(ncid, "vals", NC_INT, 1, &dimid, &varid) == NC_NOERR);

    assert(nc_enddef(ncid) == NC_NOERR);

    int written[4] = {10, 20, 30, 40};
    assert(nc_put_var_int(ncid, varid, written) == NC_NOERR);
    assert(nc_close(ncid) == NC_NOERR);

    /* Reopen + read back. */
    int rd_ncid = -1, rd_varid = -1;
    assert(nc_open(path, NC_NOWRITE, &rd_ncid) == NC_NOERR);
    assert(nc_inq_varid(rd_ncid, "vals", &rd_varid) == NC_NOERR);

    int readback[4] = {0};
    assert(nc_get_var_int(rd_ncid, rd_varid, readback) == NC_NOERR);
    assert(nc_close(rd_ncid) == NC_NOERR);

    for (int i = 0; i < 4; i++) {
        assert(readback[i] == written[i]);
    }

    unlink(path);

    printf("netcdf_c test OK: %s\n", v);
    return 0;
}
