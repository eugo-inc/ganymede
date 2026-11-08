/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for highway: aligned nd array; calls hwy::VectorBytes(), hwy::IsAligned().
 */

#include <hwy/aligned_allocator.h>
#include <hwy/base.h>
#include <hwy/per_target.h>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>

int main() {
    constexpr size_t kRows = 4;
    constexpr size_t kCols = 7;  /* deliberately not a vector-multiple */

    /* 1. Construct a 4x7 float array. The constructor zero-fills + pads. */
    hwy::AlignedNDArray<float, 2> arr({kRows, kCols});

    /* 2. Logical shape echoes back exactly what was requested. */
    const std::array<size_t, 2> &shape = arr.shape();
    assert(shape[0] == kRows);
    assert(shape[1] == kCols);

    /* 3. memory_shape() must pad the innermost (cols) axis up to a multiple
     * of VectorBytes()/sizeof(float). The outer axis is unchanged. */
    const std::array<size_t, 2> &mem_shape = arr.memory_shape();
    assert(mem_shape[0] == kRows);
    /* memory cols must be >= logical cols, and rounded up to VectorBytes
     * granularity (VectorBytes is in BYTES, padding is computed via
     * RoundUpTo(cols, VectorBytes()) in the header — so the padded cols
     * count is a multiple of VectorBytes()). */
    assert(mem_shape[1] >= kCols);
    const size_t vbytes = hwy::VectorBytes();
    assert(vbytes > 0);
    assert(mem_shape[1] % vbytes == 0);

    /* 4. The buffer pointer must be HWY_ALIGNMENT-aligned (AllocateAligned
     * contract). Each row lands on a VectorBytes()-aligned boundary because
     * the innermost axis is padded to a multiple of VectorBytes() — that's
     * enough for the per-row SIMD load contract (HWY_ALIGNMENT may be
     * larger than VectorBytes() on platforms with small vector width). */
    float *data = arr.data();
    assert(data != nullptr);
    assert(hwy::IsAligned(data));  /* base is HWY_ALIGNMENT-aligned */
    /* Row 1 = data + mem_shape[1] floats. Must be aligned to VectorBytes(). */
    assert(hwy::IsAligned(data + mem_shape[1], vbytes));

    /* 5. The constructor zero-fills the entire (padded) buffer. memory_size()
     * is the total padded element count; spot-check a few entries. */
    const size_t total = arr.memory_size();
    assert(total == mem_shape[0] * mem_shape[1]);
    assert(data[0] == 0.0f);
    assert(data[total - 1] == 0.0f);

    hwy::Span<float> row0 = arr[{0}];
    assert(row0.size() == kCols);  /* logical width, not padded */
    assert(row0.data() == data);  /* row 0 starts at buffer base */
    for (size_t i = 0; i < kCols; ++i) {
        row0[i] = static_cast<float>(i + 1);
    }
    /* Confirm via raw pointer that the writes landed. */
    for (size_t i = 0; i < kCols; ++i) {
        assert(data[i] == static_cast<float>(i + 1));
    }

    /* 7. Row 2 is independent (different offset) and starts zero. */
    hwy::Span<float> row2 = arr[{2}];
    assert(row2.data() == data + 2 * mem_shape[1]);
    assert(row2[0] == 0.0f);
    assert(row2[kCols - 1] == 0.0f);

    /* 8. truncate() shrinks logical shape; memory_shape stays the same. */
    arr.truncate({kRows - 1, kCols - 1});
    assert(arr.shape()[0] == kRows - 1);
    assert(arr.shape()[1] == kCols - 1);
    /* memory_shape is untouched — buffer not reallocated. */
    assert(arr.memory_shape()[0] == kRows);
    assert(arr.memory_shape()[1] == mem_shape[1]);
    assert(data[0] == 1.0f);

    std::printf("highway AlignedNDArray test OK: shape=%zux%zu mem=%zux%zu vbytes=%zu\n",
                kRows, kCols, mem_shape[0], mem_shape[1], vbytes);
    return 0;
}
