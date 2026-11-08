/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: int128; calls absl::MakeUint128(), absl::Uint128High64(), absl::Uint128Low64(), absl::StrCat().
 */

#include <absl/base/config.h>
#include <absl/numeric/int128.h>
#include <absl/strings/str_cat.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <string>

int main() {
    const uint64_t hi = 0x0123456789ABCDEFULL;
    const uint64_t lo = 0xFEDCBA9876543210ULL;
    /* (1) MakeUint128 round-trip through the High64 / Low64 accessors. */
    const absl::uint128 v = absl::MakeUint128(hi, lo);
    assert(absl::Uint128High64(v) == hi);
    assert(absl::Uint128Low64(v) == lo);

    /* (2) 64x64 -> 128 multiplication without overflow.
     * (2^63) * 2 == 2^64 == MakeUint128(1, 0) — exact carry into the upper half. */
    const absl::uint128 a = static_cast<absl::uint128>(1ULL << 63);
    const absl::uint128 b = static_cast<absl::uint128>(2ULL);
    const absl::uint128 product = a * b;
    assert(absl::Uint128High64(product) == 1ULL);
    assert(absl::Uint128Low64(product) == 0ULL);
    assert(product == absl::MakeUint128(1, 0));

    /* uint128 addition + comparison. */
    const absl::uint128 one = 1;
    const absl::uint128 max_u64 = static_cast<absl::uint128>(UINT64_MAX);
    const absl::uint128 max_u64_plus_one = max_u64 + one;
    assert(absl::Uint128High64(max_u64_plus_one) == 1ULL);
    assert(absl::Uint128Low64(max_u64_plus_one) == 0ULL);
    assert(max_u64_plus_one > max_u64);

    /* (3) int128 signed: negation + comparison. */
    const absl::int128 pos = 12345;
    const absl::int128 neg = -pos;
    assert(neg < 0);
    assert(pos > 0);
    assert(neg + pos == 0);
    assert(-neg == pos);

    /* (4) StrCat support for uint128 via AbslStringify. uint128 gained AbslStringify in
     * the 20230802 LTS release, so older LTS releases skip this step. Non-LTS builds
     * leave ABSL_LTS_RELEASE_VERSION undefined and always run it. */
#if !defined(ABSL_LTS_RELEASE_VERSION) || ABSL_LTS_RELEASE_VERSION >= 20230802
    const absl::uint128 small = 42;
    const std::string repr = absl::StrCat(small);
    assert(repr == "42");
#endif

    std::printf("absl int128 test OK: product_hi=%llu product_lo=%llu\n",
                static_cast<unsigned long long>(absl::Uint128High64(product)),
                static_cast<unsigned long long>(absl::Uint128Low64(product)));
    return 0;
}
