/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hdf5: group; calls H5Pcreate(), H5Pset_fapl_core(), H5Fcreate(), H5Gcreate2().
 */

#include <hdf5.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    assert(H5Pset_fapl_core(fapl, 4096, 0) >= 0);

    hid_t file = H5Fcreate("demo_hdf5_group.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    assert(file >= 0);

    /* Create nested groups one by one — exercises H5Gcreate2 + link mgmt. */
    hid_t a = H5Gcreate2(file, "/a", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(a >= 0);
    hid_t b = H5Gcreate2(a, "b", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(b >= 0);
    hid_t c = H5Gcreate2(b, "c", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(c >= 0);

    /* Tiny dataset at /a/b/c/leaf. */
    hsize_t dims[1] = {3};
    hid_t space = H5Screate_simple(1, dims, NULL);
    assert(space >= 0);
    hid_t dset = H5Dcreate2(c, "leaf", H5T_NATIVE_INT, space,
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    assert(dset >= 0);
    int wbuf[3] = {7, 8, 9};
    assert(H5Dwrite(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wbuf) >= 0);
    assert(H5Dclose(dset) >= 0);
    assert(H5Sclose(space) >= 0);

    assert(H5Gclose(c) >= 0);
    assert(H5Gclose(b) >= 0);
    assert(H5Gclose(a) >= 0);

    /* Verify each path component exists, then the deep object exists. */
    htri_t l_a = H5Lexists(file, "/a", H5P_DEFAULT);
    assert(l_a > 0);
    htri_t l_ab = H5Lexists(file, "/a/b", H5P_DEFAULT);
    assert(l_ab > 0);
    htri_t l_abc = H5Lexists(file, "/a/b/c", H5P_DEFAULT);
    assert(l_abc > 0);

    htri_t o_leaf = H5Oexists_by_name(file, "/a/b/c/leaf", H5P_DEFAULT);
    assert(o_leaf > 0);

    /* A path that should NOT exist returns 0, not negative — distinguishes
     * "absent" from "error". */
    htri_t l_missing = H5Lexists(file, "/a/b/nope", H5P_DEFAULT);
    assert(l_missing == 0);

    /* Query info on the deep group via H5Oget_info_by_name3 (1.12+ API). */
    H5O_info2_t oi;
    assert(H5Oget_info_by_name3(file, "/a/b/c", &oi, H5O_INFO_BASIC, H5P_DEFAULT) >= 0);
    assert(oi.type == H5O_TYPE_GROUP);

    assert(H5Fclose(file) >= 0);
    assert(H5Pclose(fapl) >= 0);

    printf("hdf5 group test OK\n");
    return 0;
}
