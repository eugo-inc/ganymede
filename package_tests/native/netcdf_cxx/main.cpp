/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for netcdf_cxx: core API; calls NcFile(), NcFile::addDim(), NcFile::addVar(), NcVar::putVar(), NcVar::getVar().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncFile.h>
#include <ncDim.h>
#include <ncVar.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

int main() {
    char path[] = "/tmp/demo_netcdf_cxx_test_XXXXXX.nc";
    int fd = mkstemps(path, 3);
    assert(fd >= 0);
    close(fd);

    const int N = 4;
    const int written[N] = {10, 20, 30, 40};

    /* Write phase: open, define dim + var, put values. Scoped close on dtor. */
    {
        netCDF::NcFile nc(path, netCDF::NcFile::replace);
        netCDF::NcDim dim = nc.addDim("n", N);
        netCDF::NcVar var = nc.addVar("vals", netCDF::ncInt, dim);
        var.putVar(written);
    }

    /* Read phase: reopen, fetch var, getVar values, compare. */
    int readback[N] = {0};
    {
        netCDF::NcFile nc(path, netCDF::NcFile::read);
        netCDF::NcVar var = nc.getVar("vals");
        assert(!var.isNull());
        var.getVar(readback);
    }

    for (int i = 0; i < N; ++i) {
        assert(readback[i] == written[i]);
    }

    unlink(path);

    std::printf("netcdf_cxx test OK\n");
    return 0;
}
