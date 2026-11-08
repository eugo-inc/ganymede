/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for imath: quat; calls Imath::Quatf::setAxisAngle(), Imath::Quatf::rotateVector(), Imath::Quatf::inverse().
 */

#include <Imath/ImathQuat.h>
#include <Imath/ImathVec.h>
#include <Imath/ImathMath.h>
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    /* Identity quaternion: real=1, imaginary=(0,0,0). */
    Imath::Quatf identity;
    assert(std::fabs(identity.r - 1.0f) < 1e-6f);
    assert(std::fabs(identity.v.x - 0.0f) < 1e-6f);
    assert(std::fabs(identity.v.y - 0.0f) < 1e-6f);
    assert(std::fabs(identity.v.z - 0.0f) < 1e-6f);

    /* Identity has length 1. */
    assert(std::fabs(identity.length() - 1.0f) < 1e-6f);

    Imath::Quatf q;
    q.setAxisAngle(Imath::V3f(0.0f, 0.0f, 1.0f), static_cast<float>(M_PI / 2.0));
    /* Resulting quat should be (cos(pi/4), 0, 0, sin(pi/4)) ~= (0.7071, 0, 0, 0.7071). */
    assert(std::fabs(q.r - std::cos(static_cast<float>(M_PI / 4.0))) < 1e-5f);
    assert(std::fabs(q.v.z - std::sin(static_cast<float>(M_PI / 4.0))) < 1e-5f);

    /* Unit quaternion: length == 1. */
    assert(std::fabs(q.length() - 1.0f) < 1e-5f);

    /* Rotating the +X axis 90deg about Z should yield +Y. */
    Imath::V3f x_axis(1.0f, 0.0f, 0.0f);
    Imath::V3f rotated = q.rotateVector(x_axis);
    assert(std::fabs(rotated.x - 0.0f) < 1e-5f);
    assert(std::fabs(rotated.y - 1.0f) < 1e-5f);
    assert(std::fabs(rotated.z - 0.0f) < 1e-5f);

    /* Conjugate of a unit quaternion = its inverse rotation. */
    Imath::Quatf q_inv = q.inverse();
    Imath::V3f back = q_inv.rotateVector(rotated);
    assert(std::fabs(back.x - 1.0f) < 1e-5f);
    assert(std::fabs(back.y - 0.0f) < 1e-5f);
    assert(std::fabs(back.z - 0.0f) < 1e-5f);

    /* Composing q * q_inv should yield (close to) identity. */
    Imath::Quatf composed = q * q_inv;
    assert(std::fabs(composed.r - 1.0f) < 1e-5f);
    assert(std::fabs(composed.v.x - 0.0f) < 1e-5f);
    assert(std::fabs(composed.v.y - 0.0f) < 1e-5f);
    assert(std::fabs(composed.v.z - 0.0f) < 1e-5f);

    std::printf("imath quat test OK\n");
    return 0;
}
