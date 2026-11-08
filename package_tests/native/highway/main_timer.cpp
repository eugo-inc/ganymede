/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for highway: timer; calls hwy::platform::InvariantTicksPerSecond(), hwy::platform::Now(), hwy::Unpredictable1().
 */

#include <hwy/nanobenchmark.h>
#include <hwy/timer.h>
#include <cassert>
#include <cstdint>
#include <cstdio>

int main() {
    /* 1. InvariantTicksPerSecond — exported by libhwy.so, must be > 0. */
    const double tps = hwy::platform::InvariantTicksPerSecond();
    assert(tps > 0.0);
    assert(tps >= 1000.0);

    /* 2. platform::Now() must be monotonic. Two calls back-to-back may return
     * equal values on a coarse clock, but the second must NEVER be earlier. */
    const double t0 = hwy::platform::Now();
    /* Force a non-trivial volatile loop so the optimizer cannot collapse the
     * delta to zero; use Unpredictable1 to launder the iteration count. */
    volatile uint64_t acc = 0;
    const int loops = 100000 * hwy::Unpredictable1();
    for (int i = 0; i < loops; ++i) {
        acc += static_cast<uint64_t>(i);
    }
    const double t1 = hwy::platform::Now();
    assert(t1 >= t0);
    /* Sanity: should be < 10 seconds — a sleeping clock would catch this. */
    assert((t1 - t0) < 10.0);
    /* Volatile sink so the loop above isn't elided. */
    assert(acc != 0);

    /* 3. Unpredictable1 — must return 1 to be useful as a compiler-opaque
     * multiplier. Anything else corrupts every consumer that uses it. */
    assert(hwy::Unpredictable1() == 1);

    std::printf("highway timer test OK: ticks_per_sec=%.0f elapsed=%.6fs\n",
                tps, t1 - t0);
    return 0;
}
