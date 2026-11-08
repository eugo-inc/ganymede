/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for arm_compute_library: core API; compiles against <arm_compute/core/TensorShape.h>, <arm_compute/core/TensorInfo.h>, <arm_compute/core/Types.h>.
 */

#include <arm_compute/core/TensorShape.h>
#include <arm_compute/core/TensorInfo.h>
#include <arm_compute/core/Types.h>
#include <cassert>
#include <iostream>

int main() {
    using namespace arm_compute;

    TensorShape shape(16U, 16U, 3U, 2U);
    assert(shape.num_dimensions() == 4);
    assert(shape.total_size() == 16U * 16U * 3U * 2U);
    assert(shape[0] == 16U);
    assert(shape[3] == 2U);

    /* TensorInfo links against libarm_compute.so — ctor is out-of-line. */
    TensorInfo info(shape, 1, DataType::F32);
    assert(info.data_type() == DataType::F32);
    assert(info.num_channels() == 1);
    assert(info.tensor_shape().total_size() == shape.total_size());
    assert(info.is_resizable());

    std::cout << "arm_compute_library test OK: "
              << info.tensor_shape().total_size() << " elements\n";
    return 0;
}
