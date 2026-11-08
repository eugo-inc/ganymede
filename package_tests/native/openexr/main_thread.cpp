/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openexr: thread; calls IT::ThreadPool::estimateThreadCountForFileIO(), IT::ThreadPool::globalThreadPool().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <IlmThreadPool.h>
#include <cassert>
#include <iostream>

int main(void) {
    namespace IT = ILMTHREAD_NAMESPACE;

    /* 1. The static helper for thread-count estimation must return positive. */
    unsigned estimate = IT::ThreadPool::estimateThreadCountForFileIO();
    assert(estimate >= 1);

    /* 2. The global thread pool exists. Start single-threaded (the safest
     *    deterministic baseline). */
    IT::ThreadPool& pool = IT::ThreadPool::globalThreadPool();
    pool.setNumThreads(0);
    assert(pool.numThreads() == 0);

    /* 3. Scale up to 2 workers — exercises actual std::thread spawn. */
    pool.setNumThreads(2);
    assert(pool.numThreads() == 2);

    /* 4. Scale back down — exercises join + worker teardown. */
    pool.setNumThreads(0);
    assert(pool.numThreads() == 0);

    std::cout << "openexr ilmthread test OK (est=" << estimate << ")" << std::endl;
    return 0;
}
