/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for imath: box; calls Imath::Box3f::extendBy(), Imath::Box3f::intersects(), Imath::Box3f::size(), Imath::Box3f::center().
 */

#include <Imath/ImathBox.h>
#include <Imath/ImathVec.h>
#include <cassert>
#include <cmath>
#include <cstdio>

int main() {
    Imath::Box3f box;
    assert(box.isEmpty());

    /* extendBy points — the canonical way to grow an AABB. */
    box.extendBy(Imath::V3f(1.0f, 2.0f, 3.0f));
    box.extendBy(Imath::V3f(-1.0f, 5.0f, 0.0f));
    box.extendBy(Imath::V3f(4.0f, 0.0f, 2.0f));
    assert(!box.isEmpty());

    /* After extending, min/max should bracket the extreme coordinates. */
    assert(std::fabs(box.min.x - (-1.0f)) < 1e-6f);
    assert(std::fabs(box.min.y - 0.0f) < 1e-6f);
    assert(std::fabs(box.min.z - 0.0f) < 1e-6f);
    assert(std::fabs(box.max.x - 4.0f) < 1e-6f);
    assert(std::fabs(box.max.y - 5.0f) < 1e-6f);
    assert(std::fabs(box.max.z - 3.0f) < 1e-6f);

    /* size() = max - min; center() = (min + max) / 2. */
    Imath::V3f sz = box.size();
    assert(std::fabs(sz.x - 5.0f) < 1e-6f);
    assert(std::fabs(sz.y - 5.0f) < 1e-6f);
    assert(std::fabs(sz.z - 3.0f) < 1e-6f);

    Imath::V3f c = box.center();
    assert(std::fabs(c.x - 1.5f) < 1e-6f);
    assert(std::fabs(c.y - 2.5f) < 1e-6f);
    assert(std::fabs(c.z - 1.5f) < 1e-6f);

    assert(box.intersects(Imath::V3f(0.0f, 1.0f, 1.0f)));
    assert(!box.intersects(Imath::V3f(10.0f, 10.0f, 10.0f)));

    Imath::Box3f overlap(Imath::V3f(0.0f, 0.0f, 0.0f), Imath::V3f(2.0f, 2.0f, 2.0f));
    Imath::Box3f disjoint(Imath::V3f(20.0f, 20.0f, 20.0f), Imath::V3f(30.0f, 30.0f, 30.0f));
    assert(box.intersects(overlap));
    assert(!box.intersects(disjoint));

    std::printf("imath box test OK\n");
    return 0;
}
