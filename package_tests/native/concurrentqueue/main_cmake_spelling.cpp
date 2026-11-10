/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for concurrentqueue: cmake spelling; compiles against <moodycamel/concurrentqueue.h>, <moodycamel/lightweightsemaphore.h>.
 */

#include <moodycamel/concurrentqueue.h>
#include <moodycamel/lightweightsemaphore.h>

#include <cstdio>

int main() {
    moodycamel::ConcurrentQueue<long> q;
    for (long i = 0; i < 1000; ++i) q.enqueue(i);
    long v = 0, sum = 0;
    while (q.try_dequeue(v)) sum += v;
    moodycamel::LightweightSemaphore sem;
    sem.signal(2);
    const bool ok = sem.tryWait() && sem.tryWait() && !sem.tryWait();
    if (sum != 499500 || !ok) { std::printf("FAIL sum=%ld semaphore=%d\n", sum, ok); return 1; }
    std::printf("concurrentqueue <moodycamel/...> spelling OK: sum 499500, semaphore 2 signals\n");
    return 0;
}
