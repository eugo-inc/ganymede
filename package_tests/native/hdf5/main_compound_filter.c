/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hdf5: compound filter; calls H5Pcreate(), H5Pset_fapl_core(), H5Fcreate(), H5Tcreate().
 */

#include <hdf5.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

typedef struct Record {
    int id;
    double value;
} Record;

#define N_RECORDS 8

int main(void) {
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    assert(H5Pset_fapl_core(fapl, 4096, 0) >= 0);

    hid_t file = H5Fcreate("demo_hdf5_compound.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    assert(file >= 0);

    hid_t ctype = H5Tcreate(H5T_COMPOUND, sizeof(Record));
    assert(ctype >= 0);
    assert(H5Tinsert(ctype, "id", offsetof(Record, id), H5T_NATIVE_INT) >= 0);
    assert(H5Tinsert(ctype, "value", offsetof(Record, value), H5T_NATIVE_DOUBLE) >= 0);

    /* Verify the compound is well-formed: 2 members, total size as expected. */
    int nmembers = H5Tget_nmembers(ctype);
    assert(nmembers == 2);
    assert(H5Tget_size(ctype) == sizeof(Record));

    hsize_t dims[1] = {N_RECORDS};
    hid_t space = H5Screate_simple(1, dims, NULL);
    assert(space >= 0);

    /* Chunked layout + gzip filter on the dataset creation property list. */
    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    assert(dcpl >= 0);
    hsize_t chunk_dims[1] = {4};
    assert(H5Pset_chunk(dcpl, 1, chunk_dims) >= 0);
    assert(H5Pset_deflate(dcpl, 6) >= 0);

    hsize_t got_chunk[1] = {0};
    int chunk_rank = H5Pget_chunk(dcpl, 1, got_chunk);
    assert(chunk_rank == 1);
    assert(got_chunk[0] == 4);

    /* gzip filter must be present on the DCPL. */
    int nfilters = H5Pget_nfilters(dcpl);
    assert(nfilters >= 1);
    int found_deflate = 0;
    for (int i = 0; i < nfilters; ++i) {
        unsigned flags = 0;
        size_t cd_nelmts = 0;
        H5Z_filter_t fid = H5Pget_filter2(dcpl, (unsigned)i, &flags, &cd_nelmts,
                                          NULL, 0, NULL, NULL);
        if (fid == H5Z_FILTER_DEFLATE) {
            found_deflate = 1;
            break;
        }
    }
    assert(found_deflate == 1);

    hid_t dset = H5Dcreate2(file, "records", ctype, space,
                            H5P_DEFAULT, dcpl, H5P_DEFAULT);
    assert(dset >= 0);

    Record wbuf[N_RECORDS];
    for (int i = 0; i < N_RECORDS; ++i) {
        wbuf[i].id = i;
        wbuf[i].value = (double)i * 1.5;
    }
    assert(H5Dwrite(dset, ctype, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf) >= 0);

    Record rbuf[N_RECORDS];
    memset(rbuf, 0, sizeof(rbuf));
    assert(H5Dread(dset, ctype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rbuf) >= 0);

    for (int i = 0; i < N_RECORDS; ++i) {
        assert(rbuf[i].id == wbuf[i].id);
        assert(rbuf[i].value == wbuf[i].value);
    }

    assert(H5Dclose(dset) >= 0);
    assert(H5Pclose(dcpl) >= 0);
    assert(H5Sclose(space) >= 0);
    assert(H5Tclose(ctype) >= 0);
    assert(H5Fclose(file) >= 0);
    assert(H5Pclose(fapl) >= 0);

    printf("hdf5 compound+filter test OK\n");
    return 0;
}
