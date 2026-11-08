/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cuda_cccl: versions; compiles against <cub/version.cuh>, <thrust/version.h>, <cuda/std/type_traits>.
 */

#include <cub/version.cuh>
#include <thrust/version.h>
#include <cuda/std/type_traits>

static_assert(THRUST_VERSION > 100000, "implausible thrust version");
static_assert(CUB_VERSION > 100000, "implausible cub version");
static_assert(cuda::std::is_same_v<int, int>, "libcu++ host-side sanity");

int main() { return 0; }
