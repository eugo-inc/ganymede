/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hdf5: parallel; calls MPI_Init(), H5Pcreate(), H5Pset_fapl_mpio(), H5Fcreate().
 */

#include <hdf5.h>
#include <mpi.h>
#include <stdio.h>

#ifndef H5_HAVE_PARALLEL
#error "HDF5 was built without MPI (H5_HAVE_PARALLEL is not defined)"
#endif

int main(int argc, char **argv) {
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) { fprintf(stderr, "MPI_Init failed\n"); return 1; }
    const char *path = "demo_hdf5_parallel.h5";
    int bad = 0, buf[32], back[32] = {0};
    for (int i = 0; i < 32; i++) buf[i] = 3 * i + 1;

    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    if (H5Pset_fapl_mpio(fapl, MPI_COMM_WORLD, MPI_INFO_NULL) < 0) bad = 1;
    hid_t file = H5Fcreate(path, H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    if (file < 0 || H5Pget_driver(fapl) != H5FD_MPIO) bad = 1;
    hsize_t dims[1] = {32};
    hid_t space = H5Screate_simple(1, dims, NULL);
    hid_t dset = H5Dcreate2(file, "v", H5T_NATIVE_INT, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    hid_t dxpl = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(dxpl, H5FD_MPIO_COLLECTIVE);
    if (H5Dwrite(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, buf) < 0) bad = 1;
    if (H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, dxpl, back) < 0) bad = 1;
    for (int i = 0; i < 32; i++) if (back[i] != buf[i]) bad = 1;
    H5Pclose(dxpl); H5Dclose(dset); H5Sclose(space); H5Fclose(file); H5Pclose(fapl);
    remove(path);
    MPI_Finalize();
    printf("%s: parallel HDF5 (mpio driver, collective write + read of 32 ints, 1 process)\n", bad ? "FAIL" : "PASS");
    return bad;
}
