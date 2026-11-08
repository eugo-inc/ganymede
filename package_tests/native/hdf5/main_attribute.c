/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for hdf5: attribute; calls H5Pcreate(), H5Pset_fapl_core(), H5Fcreate(), H5Tcopy().
 */

#include <hdf5.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    hid_t fapl = H5Pcreate(H5P_FILE_ACCESS);
    assert(fapl >= 0);
    assert(H5Pset_fapl_core(fapl, 4096, 0) >= 0);

    hid_t file = H5Fcreate("demo_hdf5_attr.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl);
    assert(file >= 0);

    /* Fixed-length string datatype (16 bytes incl. NUL). */
    hid_t strtype = H5Tcopy(H5T_C_S1);
    assert(strtype >= 0);
    assert(H5Tset_size(strtype, 16) >= 0);
    assert(H5Tset_strpad(strtype, H5T_STR_NULLTERM) >= 0);

    /* Scalar dataspace for a single-value attribute. */
    hid_t space = H5Screate(H5S_SCALAR);
    assert(space >= 0);

    /* Attach to the root group. */
    hid_t root = H5Gopen2(file, "/", H5P_DEFAULT);
    assert(root >= 0);

    hid_t attr = H5Acreate2(root, "title", strtype, space, H5P_DEFAULT, H5P_DEFAULT);
    assert(attr >= 0);

    const char wbuf[16] = "demo-hdf5-attr";
    assert(H5Awrite(attr, strtype, wbuf) >= 0);
    assert(H5Aclose(attr) >= 0);

    /* Re-open by name and verify the value survived. */
    hid_t attr2 = H5Aopen(root, "title", H5P_DEFAULT);
    assert(attr2 >= 0);

    /* Attribute should be discoverable by H5Aexists too. */
    htri_t exists = H5Aexists(root, "title");
    assert(exists > 0);

    char rbuf[16] = {0};
    assert(H5Aread(attr2, strtype, rbuf) >= 0);
    assert(memcmp(wbuf, rbuf, sizeof(wbuf)) == 0);

    assert(H5Aclose(attr2) >= 0);
    assert(H5Gclose(root) >= 0);
    assert(H5Sclose(space) >= 0);
    assert(H5Tclose(strtype) >= 0);
    assert(H5Fclose(file) >= 0);
    assert(H5Pclose(fapl) >= 0);

    printf("hdf5 attribute test OK\n");
    return 0;
}
