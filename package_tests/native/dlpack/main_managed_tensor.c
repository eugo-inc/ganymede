/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for dlpack: managed tensor; compiles against <dlpack/dlpack.h>.
 */

#include <dlpack/dlpack.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int g_legacy_deleter_calls = 0;
static int g_versioned_deleter_calls = 0;

static void legacy_deleter(struct DLManagedTensor *self) {
    assert(self != NULL);
    assert(self->manager_ctx == (void *)(uintptr_t)0xDEADBEEF);
    g_legacy_deleter_calls++;
}

static void versioned_deleter(struct DLManagedTensorVersioned *self) {
    assert(self != NULL);
    assert(self->manager_ctx == (void *)(uintptr_t)0xCAFEBABE);
    g_versioned_deleter_calls++;
}

int main(void) {
    int64_t shape[1] = { 4 };
    int32_t data[4] = { 10, 20, 30, 40 };

    DLDevice cpu = { .device_type = kDLCPU, .device_id = 0 };
    DLDataType i32 = { .code = (uint8_t)kDLInt, .bits = 32, .lanes = 1 };

    /* 1. Legacy DLManagedTensor — invoke its deleter manually and verify
     *    the callback chain runs exactly once. */
    DLManagedTensor legacy = {
        .dl_tensor = {
            .data = data,
            .device = cpu,
            .ndim = 1,
            .dtype = i32,
            .shape = shape,
            .strides = NULL,
            .byte_offset = 0,
        },
        .manager_ctx = (void *)(uintptr_t)0xDEADBEEF,
        .deleter = legacy_deleter,
    };
    assert(legacy.dl_tensor.ndim == 1);
    assert(legacy.deleter != NULL);
    legacy.deleter(&legacy);
    assert(g_legacy_deleter_calls == 1);

    /* 2. DLManagedTensorVersioned — version + flags + dl_tensor + deleter.
     *    The header documents that `version` should equal the producer's
     *    DLPACK_*_VERSION at construction time. */
    struct DLManagedTensorVersioned versioned = {
        .version = { .major = DLPACK_MAJOR_VERSION, .minor = DLPACK_MINOR_VERSION },
        .manager_ctx = (void *)(uintptr_t)0xCAFEBABE,
        .deleter = versioned_deleter,
        .flags = DLPACK_FLAG_BITMASK_READ_ONLY | DLPACK_FLAG_BITMASK_IS_COPIED,
        .dl_tensor = {
            .data = data,
            .device = cpu,
            .ndim = 1,
            .dtype = i32,
            .shape = shape,
            .strides = NULL,
            .byte_offset = 0,
        },
    };
    assert(versioned.version.major == DLPACK_MAJOR_VERSION);
    assert(versioned.version.minor == DLPACK_MINOR_VERSION);
    assert((versioned.flags & DLPACK_FLAG_BITMASK_READ_ONLY) != 0);
    assert((versioned.flags & DLPACK_FLAG_BITMASK_IS_COPIED) != 0);
    /* Bit positions are documented: READ_ONLY=bit0, IS_COPIED=bit1,
     * IS_SUBBYTE_TYPE_PADDED=bit2. */
    assert(DLPACK_FLAG_BITMASK_READ_ONLY == (1UL << 0UL));
    assert(DLPACK_FLAG_BITMASK_IS_COPIED == (1UL << 1UL));
    assert(DLPACK_FLAG_BITMASK_IS_SUBBYTE_TYPE_PADDED == (1UL << 2UL));

    versioned.deleter(&versioned);
    assert(g_versioned_deleter_calls == 1);

    printf("dlpack managed-tensor test OK: legacy_deleter=%d versioned_deleter=%d\n",
           g_legacy_deleter_calls, g_versioned_deleter_calls);
    return 0;
}
