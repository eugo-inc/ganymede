/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cuda_cccl: thrust host; calls thrust::sequence(), thrust::reduce().
 */

#define THRUST_DEVICE_SYSTEM THRUST_DEVICE_SYSTEM_CPP
#include <cstdio>
#include <thrust/host_vector.h>
#include <thrust/reduce.h>
#include <thrust/sequence.h>

int main() {
    thrust::host_vector<int> values(100);
    thrust::sequence(values.begin(), values.end());  // 0..99
    const int total = thrust::reduce(values.begin(), values.end(), 0);
    if (total != 4950) {
        std::fprintf(stderr, "thrust::reduce = %d, want 4950\n", total);
        return 1;
    }
    std::printf("thrust host reduce OK\n");
    return 0;
}
