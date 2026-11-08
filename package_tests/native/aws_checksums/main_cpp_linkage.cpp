/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_checksums: cpp linkage; calls aws_checksums_crc32(), aws_checksums_crc32c(), aws_checksums_crc64nvme(), aws_checksums_crc32_ex().
 */

#include <aws/checksums/crc.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>

int main() {
    const uint8_t input[] = "123456789";
    const int len = static_cast<int>(sizeof(input) - 1);

    const uint32_t c32 = aws_checksums_crc32(input, len, 0);
    const uint32_t c32c = aws_checksums_crc32c(input, len, 0);
    const uint64_t c64 = aws_checksums_crc64nvme(input, len, 0);

    assert(c32 == 0xCBF43926u);
    assert(c32c == 0xE3069283u);
    assert(c64 == 0xAE8B14860A799888ULL);

    const uint32_t c32_ex = aws_checksums_crc32_ex(input, static_cast<std::size_t>(len), 0);
    assert(c32_ex == c32);

    std::printf("aws_checksums C++ linkage test OK: "
                "crc32=0x%08x crc32c=0x%08x crc64=0x%016llx\n",
                c32, c32c, static_cast<unsigned long long>(c64));
    return 0;
}
