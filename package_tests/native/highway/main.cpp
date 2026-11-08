/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for highway: core API; calls hwy::TargetName(), hwy::SupportedTargets().
 */

#include <hwy/highway.h>
#include <hwy/targets.h>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    /* HWY_STATIC_TARGET is a compile-time macro that names the target Highway baked in for
     * this translation unit. TargetName(...) lives in libhwy.so — calling it forces a real
     * symbol resolution against the shared library. */
    const char *static_name = hwy::TargetName(HWY_STATIC_TARGET);
    assert(static_name != nullptr);
    assert(std::strlen(static_name) > 0);

    const int64_t supported = hwy::SupportedTargets();
    assert(supported != 0);

    std::printf("highway test OK: static_target=%s supported_mask=0x%llx\n",
                static_name, (unsigned long long)supported);
    return 0;
}
