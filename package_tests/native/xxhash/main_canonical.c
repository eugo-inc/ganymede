/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xxhash: canonical; calls XXH64_canonicalFromHash(), XXH64_hashFromCanonical(), XXH128_canonicalFromHash(), XXH128_hashFromCanonical().
 */

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* ABI sanity: the canonical types are fixed-size byte arrays. */
    assert(sizeof(XXH64_canonical_t) == 8);
    assert(sizeof(XXH128_canonical_t) == 16);

    const XXH64_hash_t original64 = 0x0123456789ABCDEFULL;
    XXH64_canonical_t canon64;
    XXH64_canonicalFromHash(&canon64, original64);

    assert(canon64.digest[0] == 0x01);
    assert(canon64.digest[1] == 0x23);
    assert(canon64.digest[2] == 0x45);
    assert(canon64.digest[3] == 0x67);
    assert(canon64.digest[4] == 0x89);
    assert(canon64.digest[5] == 0xAB);
    assert(canon64.digest[6] == 0xCD);
    assert(canon64.digest[7] == 0xEF);

    XXH64_hash_t round_tripped64 = XXH64_hashFromCanonical(&canon64);
    assert(round_tripped64 == original64);

    static const char data[] =
        "canonical round-trip payload for the XXH128 codec path";
    const size_t n = sizeof(data) - 1;

    XXH128_hash_t original128 = XXH128(data, n, 0);
    XXH128_canonical_t canon128;
    XXH128_canonicalFromHash(&canon128, original128);
    XXH128_hash_t round_tripped128 = XXH128_hashFromCanonical(&canon128);
    assert(XXH128_isEqual(round_tripped128, original128));

    /* And the analogous big-endian check for the 128-bit form: the high64
     * is laid down first (bytes 0..7), then the low64 (bytes 8..15). */
    XXH128_hash_t known128 = { /* .low64  = */ 0x1122334455667788ULL,
                               /* .high64 = */ 0xAABBCCDDEEFF0011ULL };
    XXH128_canonical_t canon_known;
    XXH128_canonicalFromHash(&canon_known, known128);
    assert(canon_known.digest[0] == 0xAA);  /* high64 MSB first */
    assert(canon_known.digest[7] == 0x11);  /* high64 LSB */
    assert(canon_known.digest[8] == 0x11);  /* low64 MSB */
    assert(canon_known.digest[15] == 0x88); /* low64 LSB */

    printf("xxhash canonical test OK\n");
    return 0;
}
