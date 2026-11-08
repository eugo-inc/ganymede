/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: workers; calls abigail::workers::get_number_of_threads(), abigail::workers::queue::schedule_task().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-workers.h>
#include <cassert>
#include <atomic>
#include <iostream>
#include <memory>

namespace {

/// A trivial task that bumps a shared counter when run.
struct counting_task : public abigail::workers::task {
    std::atomic<int>* counter;
    explicit counting_task(std::atomic<int>* c) : counter(c) {}
    void perform() override { counter->fetch_add(1); }
};

}  // namespace

int main(void) {
    const size_t n_threads = abigail::workers::get_number_of_threads();
    assert(n_threads > 0);

    abigail::workers::queue q(1);
    assert(q.get_size() == 0);

    std::atomic<int> counter(0);
    const int N = 4;
    for (int i = 0; i < N; ++i) {
        abigail::workers::task_sptr t(new counting_task(&counter));
        const bool scheduled = q.schedule_task(t);
        assert(scheduled);
    }

    /* Blocks until every scheduled task has run and the worker thread
       has joined. After this returns, the counter must reflect every
       perform() invocation and the completed-tasks vector must be
       populated. */
    q.wait_for_workers_to_complete();
    assert(counter.load() == N);

    const std::vector<abigail::workers::task_sptr>& done =
        q.get_completed_tasks();
    assert(done.size() == static_cast<size_t>(N));

    std::cout << "libabigail workers test OK: n_threads=" << n_threads
              << " tasks_done=" << done.size() << std::endl;
    return 0;
}
