/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for dlpack: core API; compiles against <dlpack/dlpack.h>.
 */

#include <dlpack/dlpack.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* ABI version macros must be present and sane. */
    assert(DLPACK_MAJOR_VERSION >= 1);

    /* Instantiate the primary types and exercise their fields. */
    DLDevice device = { .device_type = kDLCPU, .device_id = 0 };
    assert(device.device_type == kDLCPU);

    DLDataType dtype = { .code = (uint8_t)kDLFloat, .bits = 32, .lanes = 1 };
    assert(dtype.code == (uint8_t)kDLFloat);

    /* 2x3 row-major float32 tensor — 24 bytes per the documented formula. */
    int64_t shape[2] = { 2, 3 };
    float data[6] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f };
    DLTensor tensor = {
        .data = data,
        .device = device,
        .ndim = 2,
        .dtype = dtype,
        .shape = shape,
        .strides = NULL,
        .byte_offset = 0,
    };

    /* The header documents: size = prod(shape) * (bits * lanes + 7) / 8. */
    size_t size = 1;
    for (int32_t i = 0; i < tensor.ndim; ++i) size *= (size_t)tensor.shape[i];
    size *= (size_t)(tensor.dtype.bits * tensor.dtype.lanes + 7) / 8;
    assert(size == 24);

    printf("dlpack test OK: v%u.%u\n", DLPACK_MAJOR_VERSION, DLPACK_MINOR_VERSION);
    return 0;
}
