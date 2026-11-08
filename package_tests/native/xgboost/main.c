/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xgboost: core API; calls XGBoostVersion(), XGDMatrixCreateFromMat(), XGDMatrixNumRow(), XGDMatrixNumCol().
 */

#include <xgboost/c_api.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int major = 0, minor = 0, patch = 0;
    XGBoostVersion(&major, &minor, &patch);
    assert(major >= 1);

    const float data[2 * 3] = {
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
    };
    DMatrixHandle dmat = NULL;
    int rc = XGDMatrixCreateFromMat(data, 2, 3, 0.0f, &dmat);
    assert(rc == 0);
    assert(dmat != NULL);

    bst_ulong nrow = 0, ncol = 0;
    assert(XGDMatrixNumRow(dmat, &nrow) == 0);
    assert(XGDMatrixNumCol(dmat, &ncol) == 0);
    assert(nrow == 2);
    assert(ncol == 3);

    assert(XGDMatrixFree(dmat) == 0);

    printf("xgboost test OK: %d.%d.%d\n", major, minor, patch);
    return 0;
}
