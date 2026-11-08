/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cuda_nvtx: mark; calls nvtxMarkA().
 */

#include <nvtx3/nvToolsExt.h>

int main(void) {
    nvtxMarkA("demo-cuda_nvtx-testspec");
    return 0;
}
