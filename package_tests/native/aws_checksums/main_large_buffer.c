/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_checksums: large buffer; calls aws_checksums_crc32(), aws_checksums_crc32c(), aws_checksums_crc64nvme(), aws_checksums_crc32_ex().
 */

#include <aws/checksums/crc.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
    /* 4 KiB is comfortably above every kernel's SIMD threshold and stresses
     * multiple SIMD inner-loop iterations + the scalar tail. */
    enum { N = 4096 };
    static uint8_t buf[N];

    /* Deterministic, varied content: a simple LCG so the buffer is not all
     * zeros (zero-buffers can mask bugs in CRC kernels that XOR poorly). */
    uint32_t s = 0x12345678u;
    for (int i = 0; i < N; i++) {
        s = s * 1103515245u + 12345u;
        buf[i] = (uint8_t)(s >> 16);
    }

    /* Reference: chain through the API one byte at a time. Each call is a
     * 1-byte input that lands in the scalar tail. Sum across all N bytes
     * gives the CRC of the whole buffer with NO SIMD path executed. */
    uint32_t crc32_ref = 0, crc32c_ref = 0;
    uint64_t crc64_ref = 0;
    for (int i = 0; i < N; i++) {
        crc32_ref  = aws_checksums_crc32(&buf[i],  1, crc32_ref);
        crc32c_ref = aws_checksums_crc32c(&buf[i], 1, crc32c_ref);
        crc64_ref  = aws_checksums_crc64nvme(&buf[i], 1, crc64_ref);
    }

    /* One-shot: exercises the SIMD inner loop on the full buffer. */
    uint32_t crc32_oneshot  = aws_checksums_crc32(buf,  N, 0);
    uint32_t crc32c_oneshot = aws_checksums_crc32c(buf, N, 0);
    uint64_t crc64_oneshot  = aws_checksums_crc64nvme(buf, N, 0);

    /* The two paths MUST agree — disagreement means the SIMD kernel is
     * miscomputing, which the small-vector smoke never catches. */
    assert(crc32_oneshot  == crc32_ref);
    assert(crc32c_oneshot == crc32c_ref);
    assert(crc64_oneshot  == crc64_ref);

    /* Sanity: the CRCs must not be trivially zero on a non-zero buffer
     * (a stub kernel that returns its `previous_crc` unchanged would
     * yield 0 here, defeating the byte-chain cross-check above). */
    assert(crc32_oneshot  != 0u);
    assert(crc32c_oneshot != 0u);
    assert(crc64_oneshot  != 0ULL);

    /* Cross-check via the _ex (size_t length) entry point too — the SIMD
     * inner loop is shared, but the dispatch wrapper is distinct. */
    uint32_t crc32_ex = aws_checksums_crc32_ex(buf, (size_t)N, 0);
    assert(crc32_ex == crc32_oneshot);

    printf("aws_checksums large-buffer test OK (N=%d): "
           "crc32=0x%08x crc32c=0x%08x crc64=0x%016llx\n",
           N, crc32_oneshot, crc32c_oneshot,
           (unsigned long long)crc64_oneshot);
    return 0;
}
