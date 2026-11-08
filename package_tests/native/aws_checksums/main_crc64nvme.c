/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_checksums: crc64nvme; calls aws_checksums_crc64nvme().
 */

#include <aws/checksums/crc.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const uint8_t input[] = "123456789";
    const int len = (int)(sizeof(input) - 1); /* 9 bytes, drop the NUL */

    uint64_t crc64 = aws_checksums_crc64nvme(input, len, 0);
    assert(crc64 == 0xAE8B14860A799888ULL);

    /* Verify the previous_crc64 continuation contract: splitting the input
     * and chaining through the running CRC must match the one-shot result. */
    uint64_t partial = aws_checksums_crc64nvme(input, 5, 0);
    uint64_t chained = aws_checksums_crc64nvme(input + 5, len - 5, partial);
    assert(chained == 0xAE8B14860A799888ULL);

    uint64_t empty = aws_checksums_crc64nvme(input, 0, 0);
    assert(empty == 0ULL);

    printf("aws_checksums crc64nvme test OK: crc64=0x%016llx\n",
           (unsigned long long)crc64);
    return 0;
}
