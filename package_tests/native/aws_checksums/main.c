/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_checksums: core API; calls aws_checksums_crc32(), aws_checksums_crc32c().
 */

#include <aws/checksums/crc.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const uint8_t input[] = "123456789";
    const int len = (int)(sizeof(input) - 1); /* 9 bytes, drop the NUL */

    uint32_t crc32 = aws_checksums_crc32(input, len, 0);
    assert(crc32 == 0xCBF43926u);

    uint32_t crc32c = aws_checksums_crc32c(input, len, 0);
    assert(crc32c == 0xE3069283u);

    /* Verify incremental accumulation works: split input in two and chain. */
    uint32_t partial = aws_checksums_crc32(input, 4, 0);
    uint32_t chained = aws_checksums_crc32(input + 4, len - 4, partial);
    assert(chained == 0xCBF43926u);

    printf("aws_checksums test OK: crc32=0x%08x crc32c=0x%08x\n", crc32, crc32c);
    return 0;
}
