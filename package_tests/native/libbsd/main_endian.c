/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libbsd: endian; calls be32enc(), be32dec(), le16enc(), le16dec().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <bsd/sys/endian.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    uint8_t buf[8];

    /* be32enc writes most-significant byte first. */
    be32enc(buf, 0x11223344u);
    assert(buf[0] == 0x11);
    assert(buf[1] == 0x22);
    assert(buf[2] == 0x33);
    assert(buf[3] == 0x44);

    assert(be32dec(buf) == 0x11223344u);

    /* le16enc writes least-significant byte first. */
    le16enc(buf, (uint16_t)0xABCDu);
    assert(buf[0] == 0xCD);
    assert(buf[1] == 0xAB);
    assert(le16dec(buf) == 0xABCDu);

    /* 64-bit big-endian decode of a known 8-byte payload. */
    static const uint8_t payload[8] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
    };
    uint64_t v = be64dec(payload);
    assert(v == 0x0123456789ABCDEFULL);

    be64enc(buf, 0xFEEDFACECAFEBEEFULL);
    assert(be64dec(buf) == 0xFEEDFACECAFEBEEFULL);

    printf("libbsd endian test OK\n");
    return 0;
}
