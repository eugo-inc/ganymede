/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jpeg_xl: threads; calls JxlThreadParallelRunnerDefaultNumWorkerThreads(), JxlThreadParallelRunnerCreate(), JxlThreadParallelRunnerDestroy().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jxl/thread_parallel_runner.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Discovery helper: a sensible default thread count must be >= 1. */
    size_t default_threads = JxlThreadParallelRunnerDefaultNumWorkerThreads();
    assert(default_threads >= 1);

    void *runner = JxlThreadParallelRunnerCreate(NULL, default_threads);
    assert(runner != NULL);

    JxlThreadParallelRunnerDestroy(runner);

    /* Also exercise the explicit single-thread path. */
    void *runner_one = JxlThreadParallelRunnerCreate(NULL, 1);
    assert(runner_one != NULL);
    JxlThreadParallelRunnerDestroy(runner_one);

    printf("jpeg_xl threads test OK: default workers = %zu\n", default_threads);
    return 0;
}
