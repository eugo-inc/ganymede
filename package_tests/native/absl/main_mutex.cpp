/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: mutex; compiles against <absl/synchronization/mutex.h>, <thread>.
 */

#include <absl/synchronization/mutex.h>
#include <cassert>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
    constexpr int kNumThreads = 8;
    constexpr int kIncrementsPerThread = 10000;

    /* Allocated once and never destroyed. Recent Abseil headers make ~Mutex()
     * call an out-of-line Mutex::Dtor() when NDEBUG is not defined, and a
     * release-mode shared build does not export that symbol, so a program
     * compiled without NDEBUG that destroys a Mutex would fail to link. */
    absl::Mutex& mu = *new absl::Mutex;
    int counter = 0;

    /* (1) Contended increment under a MutexLock guard. */
    std::vector<std::thread> threads;
    threads.reserve(kNumThreads);
    for (int i = 0; i < kNumThreads; ++i) {
        threads.emplace_back([&mu, &counter]() {
            for (int j = 0; j < kIncrementsPerThread; ++j) {
                absl::MutexLock lock(&mu);
                ++counter;
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    assert(counter == kNumThreads * kIncrementsPerThread);

    /* (2) Sanity-check the explicit Lock/Unlock pair. */
    mu.Lock();
    counter = 0;
    mu.Unlock();
    assert(counter == 0);

    /* TryLock must succeed when uncontended, then yield Unlock. */
    const bool locked = mu.TryLock();
    assert(locked);
    (void)locked;
    mu.Unlock();

    std::printf("absl mutex test OK: counter=%d (threads=%d x %d)\n",
                kNumThreads * kIncrementsPerThread, kNumThreads, kIncrementsPerThread);
    return 0;
}
