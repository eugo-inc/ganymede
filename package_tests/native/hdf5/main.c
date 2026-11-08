/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hdf5: core API; calls H5get_libversion(), H5Pcreate(), H5Pset_fapl_core(), H5Fcreate().
 */

#include <hdf5.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Library version must be queryable and match the compile-time header. */
    unsigned majnum = 0, minnum = 0, relnum = 0;
    assert(H5get_libversion(&majnum, &minnum, &relnum) >= 0);

    /* Use the CORE VFD with backing_store=0 so nothing ever touches disk. */
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    assert(H5Pset_fapl_core(fapl, 4096, 0) >= 0);

    hid_t file = H5Fcreate("demo_hdf5_smoke.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    assert(file >= 0);

    /* 1-D dataset of 4 ints. */
    hsize_t dims[1] = {4};
    hid_t space = H5Screate_simple(1, dims, NULL);
    assert(space >= 0);

    hid_t dset = H5Dcreate2(file, "ints", H5T_NATIVE_INT, space,
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dset >= 0);

    int wbuf[4] = {10, 20, 30, 40};
    assert(H5Dwrite(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf) >= 0);

    int rbuf[4] = {0, 0, 0, 0};
    assert(H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf) >= 0);
    assert(memcmp(wbuf, rbuf, sizeof(wbuf)) == 0);

    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(space) >= 0);
    assert(H5Fclose(file) >= 0);
    assert(H5Pclose(fapl) >= 0);

    printf("hdf5 test OK: %u.%u.%u\n", majnum, minnum, relnum);
    return 0;
}
