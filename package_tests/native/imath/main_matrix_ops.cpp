/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for imath: matrix ops; calls Imath::M44f::determinant(), Imath::M44f::inverse(), Imath::M44f::multVecMatrix().
 */

#include <Imath/ImathMatrix.h>
#include <Imath/ImathMatrixAlgo.h>
#include <Imath/ImathVec.h>
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    /* Scale matrix: diag(2, 3, 4). Determinant should be 2*3*4 = 24. */
    Imath::M44f scale;
    scale.setScale(Imath::V3f(2.0f, 3.0f, 4.0f));
    float det = scale.determinant();
    assert(std::fabs(det - 24.0f) < 1e-5f);

    /* Inverse of diag(2,3,4) is diag(1/2, 1/3, 1/4). */
    Imath::M44f inv = scale.inverse();
    assert(std::fabs(inv[0][0] - 0.5f) < 1e-5f);
    assert(std::fabs(inv[1][1] - (1.0f / 3.0f)) < 1e-5f);
    assert(std::fabs(inv[2][2] - 0.25f) < 1e-5f);

    Imath::M44f product = scale * inv;
    assert(std::fabs(product[0][0] - 1.0f) < 1e-5f);
    assert(std::fabs(product[1][1] - 1.0f) < 1e-5f);
    assert(std::fabs(product[2][2] - 1.0f) < 1e-5f);
    assert(std::fabs(product[3][3] - 1.0f) < 1e-5f);
    assert(std::fabs(product[0][1] - 0.0f) < 1e-5f);
    assert(std::fabs(product[1][0] - 0.0f) < 1e-5f);

    /* Translation: setTranslation sets the bottom row's first 3 columns (row-major). */
    Imath::M44f T;  /* identity */
    T.setTranslation(Imath::V3f(10.0f, 20.0f, 30.0f));
    /* Applying T to a point should add the translation. multVecMatrix treats the
     * input as a point (w=1) and applies the translation. */
    Imath::V3f origin(0.0f, 0.0f, 0.0f);
    Imath::V3f translated;
    T.multVecMatrix(origin, translated);
    assert(std::fabs(translated.x - 10.0f) < 1e-5f);
    assert(std::fabs(translated.y - 20.0f) < 1e-5f);
    assert(std::fabs(translated.z - 30.0f) < 1e-5f);

    /* Identity determinant = 1. */
    Imath::M44f I;
    assert(std::fabs(I.determinant() - 1.0f) < 1e-6f);

    /* Singular matrix (all zeros) has determinant 0. */
    Imath::M44f Z(0.0f);
    assert(std::fabs(Z.determinant() - 0.0f) < 1e-6f);

    std::printf("imath matrix ops test OK\n");
    return 0;
}
