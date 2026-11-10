/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tbb: core API; calls oneapi::tbb::parallel_reduce(), oneapi::tbb::blocked_range(), scalable_malloc(), scalable_free().
 */

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_reduce.h>
#include <oneapi/tbb/scalable_allocator.h>
#include <oneapi/tbb/task_group.h>
#include <oneapi/tbb/version.h>

int main() {
    constexpr std::int64_t count = 1'000'000;
    const std::int64_t sum = oneapi::tbb::parallel_reduce(
        oneapi::tbb::blocked_range<std::int64_t>(0, count), std::int64_t{0},
        [](const oneapi::tbb::blocked_range<std::int64_t>& range, std::int64_t total) {
            for (std::int64_t i = range.begin(); i != range.end(); ++i) {
                total += i;
            }
            return total;
        },
        [](std::int64_t left, std::int64_t right) { return left + right; });
    if (sum != count * (count - 1) / 2) {
        std::fprintf(stderr, "parallel_reduce returned %lld\n", static_cast<long long>(sum));
        return 1;
    }

    void* memory = scalable_malloc(4096);
    if (memory == nullptr) {
        std::fprintf(stderr, "scalable_malloc returned NULL\n");
        return 1;
    }
    std::memset(memory, 0x5a, 4096);
    scalable_free(memory);

    int task_group_result = 0;
    oneapi::tbb::task_group group;
    group.run([&task_group_result] { task_group_result = 42; });
    group.wait();
    if (task_group_result != 42) {
        std::fprintf(stderr, "task_group task did not run: %d\n", task_group_result);
        return 1;
    }

    if (TBB_runtime_interface_version() != TBB_INTERFACE_VERSION) {
        std::fprintf(stderr, "runtime interface %d, headers %d\n", TBB_runtime_interface_version(), TBB_INTERFACE_VERSION);
        return 1;
    }

    std::printf("tbb interface %d: parallel_reduce and scalable_malloc work\n", TBB_runtime_interface_version());
    return 0;
}
