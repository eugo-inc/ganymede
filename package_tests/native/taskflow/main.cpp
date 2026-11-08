/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for taskflow: core API; compiles against <taskflow/taskflow.hpp>, <atomic>.
 */

#include <taskflow/taskflow.hpp>

#include <atomic>
#include <cstdio>

int main() {
    tf::Executor executor(2);
    tf::Taskflow taskflow("demo-native-test");

    std::atomic<int> step{0};
    int order[3] = {-1, -1, -1};

    tf::Task first = taskflow.emplace([&] { order[0] = step++; });
    tf::Task second = taskflow.emplace([&] { order[1] = step++; });
    tf::Task third = taskflow.emplace([&] { order[2] = step++; });
    first.precede(second);
    second.precede(third);

    executor.run(taskflow).wait();

    if (order[0] != 0 || order[1] != 1 || order[2] != 2) {
        std::fprintf(stderr, "tasks ran out of order: %d %d %d\n", order[0], order[1], order[2]);
        return 1;
    }

    std::printf("taskflow %d.%d.%d: A -> B -> C ran in order on %zu workers\n", TF_VERSION / 100000,
                TF_VERSION / 100 % 1000, TF_VERSION % 100, executor.num_workers());
    return 0;
}
