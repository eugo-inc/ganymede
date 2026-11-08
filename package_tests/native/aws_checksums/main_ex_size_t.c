/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_checksums: ex size t; calls aws_checksums_crc32(), aws_checksums_crc32_ex(), aws_checksums_crc32c(), aws_checksums_crc32c_ex().
 */

#include <aws/checksums/crc.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const uint8_t input[] = "123456789";
    const size_t len = sizeof(input) - 1; /* 9 bytes, drop the NUL */

    /* The _ex entry points must agree with the int-length entry points on
     * inputs both APIs can express. If they don't, the size_t dispatch is
     * broken (different inner kernel selection, byte-swap bug, etc.). */
    uint32_t crc32_int  = aws_checksums_crc32(input,  (int)len, 0);
    uint32_t crc32_size = aws_checksums_crc32_ex(input, len, 0);
    assert(crc32_int == crc32_size);
    assert(crc32_size == 0xCBF43926u);

    uint32_t crc32c_int  = aws_checksums_crc32c(input,  (int)len, 0);
    uint32_t crc32c_size = aws_checksums_crc32c_ex(input, len, 0);
    assert(crc32c_int == crc32c_size);
    assert(crc32c_size == 0xE3069283u);

    uint64_t crc64_int  = aws_checksums_crc64nvme(input,  (int)len, 0);
    uint64_t crc64_size = aws_checksums_crc64nvme_ex(input, len, 0);
    assert(crc64_int == crc64_size);
    assert(crc64_size == 0xAE8B14860A799888ULL);

    uint32_t seed_rt_32  = aws_checksums_crc32_ex(input, 0, 0xDEADBEEFu);
    assert(seed_rt_32 == 0xDEADBEEFu);
    uint32_t seed_rt_32c = aws_checksums_crc32c_ex(input, 0, 0xCAFEBABEu);
    assert(seed_rt_32c == 0xCAFEBABEu);
    uint64_t seed_rt_64  = aws_checksums_crc64nvme_ex(input, 0, 0x0123456789ABCDEFULL);
    assert(seed_rt_64 == 0x0123456789ABCDEFULL);

    /* Split-buffer chaining via the _ex (size_t) API: a separate continuation
     * code path from the int-API chaining covered in main.c. */
    uint32_t partial_32 = aws_checksums_crc32_ex(input, (size_t)4, 0);
    uint32_t chained_32 = aws_checksums_crc32_ex(input + 4, len - 4, partial_32);
    assert(chained_32 == 0xCBF43926u);

    printf("aws_checksums _ex test OK: crc32=0x%08x crc32c=0x%08x crc64=0x%016llx\n",
           crc32_size, crc32c_size, (unsigned long long)crc64_size);
    return 0;
}
