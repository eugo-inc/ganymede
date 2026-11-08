/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cuda_toolkit: runtime version; calls cudaRuntimeGetVersion(), cudaGetErrorString().
 */

#include <cuda_runtime.h>
#include <stdio.h>

int main(void) {
    int runtime_version = 0;
    cudaError_t status = cudaRuntimeGetVersion(&runtime_version);
    if (status != cudaSuccess) {
        fprintf(stderr, "cudaRuntimeGetVersion failed: %d (%s)\n", (int)status, cudaGetErrorString(status));
        return 1;
    }
    if (runtime_version <= 0) {
        fprintf(stderr, "unexpected CUDA runtime version %d (expected a positive value)\n", runtime_version);
        return 1;
    }
    printf("cuda_runtime_version=%d\n", runtime_version);
    return 0;
}
