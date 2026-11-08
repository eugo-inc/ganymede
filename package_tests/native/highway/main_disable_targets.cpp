/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for highway: disable targets; calls hwy::SupportedTargets(), hwy::SetSupportedTargetsForTest(), hwy::DisableTargets().
 */

#include <hwy/highway.h>
#include <hwy/targets.h>
#include <cassert>
#include <cstdint>
#include <cstdio>

int main() {
    const int64_t baseline = hwy::SupportedTargets();
    assert(baseline != 0);

    /* 2. SetSupportedTargetsForTest forces SupportedTargets() to return a
     * specific value (this is how Highway's own tests iterate per-target).
     * Pick an arbitrary nonzero mask: HWY_STATIC_TARGET is always set in
     * the static-target bit position, so it must echo back.
     *
     * Note: SetSupportedTargetsForTest is documented to be intersected with
     * DisableTargets() — but with no prior DisableTargets() call, the mocked
     * value passes through unchanged. */
    hwy::SetSupportedTargetsForTest(HWY_STATIC_TARGET);
    const int64_t mocked = hwy::SupportedTargets();
    /* The mocked bit must be present. */
    assert((mocked & HWY_STATIC_TARGET) != 0);

    /* 3. Restore via the documented "pass 0 to disable the mock" contract. */
    hwy::SetSupportedTargetsForTest(0);
    const int64_t restored = hwy::SupportedTargets();
    assert((restored & baseline) == baseline);

    hwy::DisableTargets(HWY_STATIC_TARGET);
    const int64_t after_disable = hwy::SupportedTargets();
    assert(after_disable != 0);
    /* Either: (a) HWY_STATIC_TARGET is no longer in the bitfield, OR
     * (b) the disable list reduced supported to empty so library fell back
     * to HWY_STATIC_TARGET (in which case after_disable == HWY_STATIC_TARGET).
     * Both are documented behaviors. */
    const bool target_dropped = (after_disable & HWY_STATIC_TARGET) == 0;
    const bool only_fallback_left = (after_disable == HWY_STATIC_TARGET);
    assert(target_dropped || only_fallback_left);

    hwy::SetSupportedTargetsForTest(baseline);
    const int64_t after_remock = hwy::SupportedTargets();
    assert(after_remock != 0);

    std::printf("highway disable-targets test OK: baseline=0x%llx after_disable=0x%llx\n",
                (unsigned long long)baseline, (unsigned long long)after_disable);
    return 0;
}
