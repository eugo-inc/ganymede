/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for dlpack: dtype codes; compiles against <dlpack/dlpack.h>.
 */

#include <dlpack/dlpack.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static size_t get_data_size(const DLTensor *t) {
    size_t size = 1;
    for (int32_t i = 0; i < t->ndim; ++i) {
        size *= (size_t)t->shape[i];
    }
    size *= (size_t)(t->dtype.bits * t->dtype.lanes + 7) / 8;
    return size;
}

int main(void) {
    /* 1. Data-type codes are fixed wire-format constants. */
    assert((unsigned)kDLInt == 0U);
    assert((unsigned)kDLUInt == 1U);
    assert((unsigned)kDLFloat == 2U);
    assert((unsigned)kDLOpaqueHandle == 3U);
    assert((unsigned)kDLBfloat == 4U);
    assert((unsigned)kDLComplex == 5U);
    assert((unsigned)kDLBool == 6U);

    /* 2. Device-type values that frameworks dispatch on. */
    assert((int)kDLCPU == 1);
    assert((int)kDLCUDA == 2);
    assert((int)kDLCUDAHost == 3);
    assert((int)kDLROCM == 10);
    assert((int)kDLCUDAManaged == 13);

    DLDevice cpu = { .device_type = kDLCPU, .device_id = 0 };

    /* 3. GetDataSize matrix — exercise the formula across bit widths and lanes. */
    int64_t shape3[1] = { 10 };

    /* int8 — 10 bytes */
    DLTensor t_i8 = {
        .data = NULL, .device = cpu, .ndim = 1,
        .dtype = { .code = (uint8_t)kDLInt, .bits = 8, .lanes = 1 },
        .shape = shape3, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_i8) == 10);

    /* float32 vec4 (lanes=4) over 10 elements — 10 * 16 = 160 bytes */
    DLTensor t_f32x4 = {
        .data = NULL, .device = cpu, .ndim = 1,
        .dtype = { .code = (uint8_t)kDLFloat, .bits = 32, .lanes = 4 },
        .shape = shape3, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_f32x4) == 160);

    /* complex64 — 10 * 8 = 80 bytes (bits=64, lanes=1) */
    DLTensor t_c64 = {
        .data = NULL, .device = cpu, .ndim = 1,
        .dtype = { .code = (uint8_t)kDLComplex, .bits = 64, .lanes = 1 },
        .shape = shape3, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_c64) == 80);

    /* bool — 10 * 1 = 10 bytes (header documents bits=8 for bool storage). */
    DLTensor t_bool = {
        .data = NULL, .device = cpu, .ndim = 1,
        .dtype = { .code = (uint8_t)kDLBool, .bits = 8, .lanes = 1 },
        .shape = shape3, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_bool) == 10);

    /* bfloat16 — 10 * 2 = 20 bytes */
    DLTensor t_bf16 = {
        .data = NULL, .device = cpu, .ndim = 1,
        .dtype = { .code = (uint8_t)kDLBfloat, .bits = 16, .lanes = 1 },
        .shape = shape3, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_bf16) == 20);

    /* 4. 0-dim scalar tensor — product over zero dims is 1; size = 1 * 4 = 4 bytes for float32. */
    DLTensor t_scalar = {
        .data = NULL, .device = cpu, .ndim = 0,
        .dtype = { .code = (uint8_t)kDLFloat, .bits = 32, .lanes = 1 },
        .shape = NULL, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_scalar) == 4);

    /* 5. 2D shape — the product runs over every dim; shape=[4,5] float32 is 80 bytes. */
    int64_t shape_2d[2] = { 4, 5 };
    DLTensor t_2d = {
        .data = NULL, .device = cpu, .ndim = 2,
        .dtype = { .code = (uint8_t)kDLFloat, .bits = 32, .lanes = 1 },
        .shape = shape_2d, .strides = NULL, .byte_offset = 0,
    };
    assert(get_data_size(&t_2d) == 80);

    printf("dlpack dtype-codes test OK: %u type codes pinned\n", (unsigned)kDLBool + 1U);
    return 0;
}
