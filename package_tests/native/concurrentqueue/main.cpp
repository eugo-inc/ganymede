/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for concurrentqueue: core API; compiles against <concurrentqueue/moodycamel/blockingconcurrentqueue.h>, <concurrentqueue/moodycamel/concurrentqueue.h>, <atomic>.
 */

#include <concurrentqueue/moodycamel/blockingconcurrentqueue.h>
#include <concurrentqueue/moodycamel/concurrentqueue.h>

#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
    constexpr int kProducers = 4, kConsumers = 4, kPerProducer = 100000, kTotal = kProducers * kPerProducer;
    moodycamel::ConcurrentQueue<int> q;
    std::vector<std::atomic<int>> seen(kTotal);
    for (auto& s : seen) s.store(0);
    std::atomic<int> taken{0};
    std::vector<std::thread> threads;
    for (int p = 0; p < kProducers; ++p)
        threads.emplace_back([&, p] { for (int i = 0; i < kPerProducer; ++i) q.enqueue(p * kPerProducer + i); });
    for (int c = 0; c < kConsumers; ++c)
        threads.emplace_back([&] {
            int v;
            while (taken.load() < kTotal) {
                if (q.try_dequeue(v)) { seen[v].fetch_add(1); taken.fetch_add(1); }
            }
        });
    for (auto& t : threads) t.join();
    for (int i = 0; i < kTotal; ++i) {
        if (seen[i].load() != 1) { std::printf("FAIL value %d seen %d times\n", i, seen[i].load()); return 1; }
    }

    moodycamel::BlockingConcurrentQueue<int> bq;
    int got = 0;
    std::thread consumer([&] { bq.wait_dequeue(got); });
    bq.enqueue(42);
    consumer.join();
    if (got != 42) { std::printf("FAIL blocking queue gave %d\n", got); return 1; }

    std::printf("concurrentqueue OK: %d values through 4x4 threads exactly once; blocking hand-off 42\n", kTotal);
    return 0;
}
