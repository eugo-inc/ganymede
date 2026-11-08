/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for dlpack: cxx abi; compiles against <dlpack/dlpack.h>.
 */

#include <dlpack/dlpack.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

static int g_versioned_deleter_calls = 0;

/* A C-ABI deleter callback matching DLManagedTensorVersioned::deleter. */
extern "C" void my_deleter(struct DLManagedTensorVersioned *self) {
    assert(self != nullptr);
    assert(self->manager_ctx == reinterpret_cast<void *>(static_cast<uintptr_t>(0xDEADBEEF)));
    g_versioned_deleter_calls++;
}

int main() {
    /* 1. Sanity: version macros are visible under C++ too. */
    static_assert(DLPACK_MAJOR_VERSION >= 1, "dlpack major version regressed");

    /* 2. Struct sizes match the documented C layout. */
    static_assert(sizeof(DLDevice) >= sizeof(int32_t) * 2,
                  "DLDevice smaller than expected (device_type + device_id)");
    static_assert(sizeof(DLDataType) == 4,
                  "DLDataType ABI: 1 + 1 + 2 = 4 bytes (code/bits/lanes)");
    static_assert(sizeof(DLPackVersion) == sizeof(uint32_t) * 2,
                  "DLPackVersion: 2 * uint32_t (major + minor)");
    /* DLTensor on a 64-bit platform: ptr + (i32+i32) + i32 + (u8+u8+u16)
     * + ptr + ptr + u64, with alignment padding. Lower-bound it. */
    static_assert(sizeof(DLTensor) >= 32,
                  "DLTensor missing fields (lower bound)");

    /* 3. The C++ "typed enum" form is in effect — its underlying type is int32. */
    static_assert(sizeof(DLDeviceType) == sizeof(int32_t),
                  "DLDeviceType must be int32-backed under C++");

    /* 4. Build a DLManagedTensorVersioned in C++ and call its C-linkage deleter. */
    int64_t shape[3] = { 2, 3, 4 };
    DLManagedTensorVersioned versioned;
    std::memset(&versioned, 0, sizeof(versioned));
    versioned.version.major = DLPACK_MAJOR_VERSION;
    versioned.version.minor = DLPACK_MINOR_VERSION;
    versioned.manager_ctx = reinterpret_cast<void *>(static_cast<uintptr_t>(0xDEADBEEF));
    versioned.deleter = my_deleter;
    versioned.flags = DLPACK_FLAG_BITMASK_READ_ONLY | DLPACK_FLAG_BITMASK_IS_COPIED;
    versioned.dl_tensor.data = nullptr;
    versioned.dl_tensor.device.device_type = kDLCPU;
    versioned.dl_tensor.device.device_id = 0;
    versioned.dl_tensor.ndim = 3;
    versioned.dl_tensor.dtype.code = static_cast<uint8_t>(kDLFloat);
    versioned.dl_tensor.dtype.bits = 32;
    versioned.dl_tensor.dtype.lanes = 1;
    versioned.dl_tensor.shape = shape;
    versioned.dl_tensor.strides = nullptr;
    versioned.dl_tensor.byte_offset = 0;

    assert(versioned.version.major == DLPACK_MAJOR_VERSION);
    assert(versioned.deleter != nullptr);
    assert((versioned.flags & DLPACK_FLAG_BITMASK_READ_ONLY) != 0);
    assert((versioned.flags & DLPACK_FLAG_BITMASK_IS_COPIED) != 0);

    /* Invoke the C-linkage deleter through the struct's function-pointer field. */
    versioned.deleter(&versioned);
    assert(g_versioned_deleter_calls == 1);

    /* 5. Mixed C++/C enum interop — DLDataTypeCode values used as uint8_t. */
    DLDataType d{};
    d.code = static_cast<uint8_t>(kDLBfloat);
    d.bits = 16;
    d.lanes = 1;
    assert(d.code == static_cast<uint8_t>(kDLBfloat));

    std::printf("dlpack cxx-abi test OK: sizeof(DLTensor)=%zu sizeof(DLManagedTensorVersioned)=%zu\n",
                sizeof(DLTensor), sizeof(DLManagedTensorVersioned));
    return 0;
}
