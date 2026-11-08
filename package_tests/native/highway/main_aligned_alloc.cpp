/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for highway: aligned alloc; calls hwy::AllocateAlignedBytes(), hwy::IsAligned(), hwy::FreeAlignedBytes(), hwy::AllocateAligned().
 */

#include <hwy/aligned_allocator.h>
#include <hwy/base.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>

int main() {
    /* 1. Raw AllocateAlignedBytes path — used by every higher-level helper. */
    void *raw = hwy::AllocateAlignedBytes(4096, nullptr, nullptr);
    assert(raw != nullptr);
    /* Highway guarantees alignment to at least HWY_ALIGNMENT (128 bytes). */
    assert(reinterpret_cast<uintptr_t>(raw) % HWY_ALIGNMENT == 0);
    assert(hwy::IsAligned(static_cast<char *>(raw)));
    /* Should still be safely writable across the requested span. */
    std::memset(raw, 0xAB, 4096);
    assert(static_cast<unsigned char *>(raw)[0] == 0xAB);
    assert(static_cast<unsigned char *>(raw)[4095] == 0xAB);
    hwy::FreeAlignedBytes(raw, nullptr, nullptr);

    /* 2. MakeUniqueAlignedArray — header-only wrapper around the runtime
     * primitives above. Confirms the deleter calls FreeAlignedBytes correctly
     * on scope exit (would leak / double-free otherwise). */
    {
        constexpr size_t kCount = 256;
        hwy::AlignedFreeUniquePtr<int32_t[]> arr =
            hwy::AllocateAligned<int32_t>(kCount);
        assert(arr.get() != nullptr);
        assert(hwy::IsAligned(arr.get()));
        for (size_t i = 0; i < kCount; ++i) {
            arr[i] = static_cast<int32_t>(i * 3 + 1);
        }
        int64_t sum = 0;
        for (size_t i = 0; i < kCount; ++i) {
            sum += arr[i];
        }
        /* Sum of (3i + 1) for i in [0, kCount) = 3 * kCount*(kCount-1)/2 + kCount. */
        const int64_t expected =
            3 * static_cast<int64_t>(kCount) * (kCount - 1) / 2 + kCount;
        assert(sum == expected);
    }  /* arr's deleter runs here. */

    std::printf("highway aligned-alloc test OK: HWY_ALIGNMENT=%d\n",
                HWY_ALIGNMENT);
    return 0;
}
