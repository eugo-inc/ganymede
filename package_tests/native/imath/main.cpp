/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for imath: core API; compiles against <Imath/ImathVec.h>, <Imath/ImathMatrix.h>, <Imath/half.h>.
 */

#include <Imath/ImathVec.h>
#include <Imath/ImathMatrix.h>
#include <Imath/half.h>
#include <Imath/ImathConfig.h>
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    /* Vector arithmetic — exercises the templated Vec3 core. */
    Imath::V3f a(1.0f, 2.0f, 3.0f);
    Imath::V3f b(4.0f, 5.0f, 6.0f);
    float dot = a.dot(b);
    assert(std::fabs(dot - 32.0f) < 1e-6f);  /* 1*4 + 2*5 + 3*6 = 32 */

    /* Identity matrix * vector = vector — exercises the M44f multVecMatrix path. */
    Imath::M44f identity;  /* default ctor = identity */
    Imath::V3f out;
    identity.multVecMatrix(a, out);
    assert(std::fabs(out.x - 1.0f) < 1e-6f);
    assert(std::fabs(out.y - 2.0f) < 1e-6f);
    assert(std::fabs(out.z - 3.0f) < 1e-6f);

    Imath::half h(1.5f);
    assert(static_cast<float>(h) == 1.5f);

    std::printf("imath test OK: IMATH_VERSION_STRING=%s\n", IMATH_VERSION_STRING);
    return 0;
}
