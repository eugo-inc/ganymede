/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libdeflate: checksum; calls libdeflate_crc32(), libdeflate_adler32().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libdeflate.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    const char *crc_input = "123456789";
    const size_t crc_input_len = strlen(crc_input);
    uint32_t crc = libdeflate_crc32(0, crc_input, crc_input_len);
    assert(crc == 0xCBF43926u);

    uint32_t crc_partial = libdeflate_crc32(0, "1234", 4);
    uint32_t crc_chained = libdeflate_crc32(crc_partial, "56789", 5);
    assert(crc_chained == 0xCBF43926u);

    /* Canonical Adler-32 test vector: Adler32("Wikipedia") == 0x11E60398. */
    const char *adler_input = "Wikipedia";
    const size_t adler_input_len = strlen(adler_input);
    uint32_t adler = libdeflate_adler32(1, adler_input, adler_input_len);
    assert(adler == 0x11E60398u);

    /* Incremental Adler-32 chaining: same result via two chunks. */
    uint32_t adler_partial = libdeflate_adler32(1, "Wiki", 4);
    uint32_t adler_chained = libdeflate_adler32(adler_partial, "pedia", 5);
    assert(adler_chained == 0x11E60398u);

    const char empty_marker = 0;
    assert(libdeflate_crc32(0xDEADBEEFu, &empty_marker, 0) == 0xDEADBEEFu);
    assert(libdeflate_adler32(0x12345u, &empty_marker, 0) == 0x12345u);

    /* NULL-buffer sentinel: libdeflate returns the canonical initial value
     * (0 for CRC32, 1 for Adler32), NOT the caller-supplied seed. */
    assert(libdeflate_crc32(0xDEADBEEFu, NULL, 0) == 0u);
    assert(libdeflate_adler32(0xDEADBEEFu, NULL, 0) == 1u);

    printf("libdeflate checksum test OK (crc32=0x%08x adler32=0x%08x)\n",
           crc, adler);
    return 0;
}
