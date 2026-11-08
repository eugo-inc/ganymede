/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for google_crc32c: core API; calls crc32c_value(), crc32c_extend().
 */

#include <crc32c/crc32c.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const uint8_t input[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    const uint32_t expected = 0xe3069283u;

    uint32_t one_shot = crc32c_value(input, sizeof(input));
    assert(one_shot == expected);

    /* Split the input and feed it through crc32c_extend(); composition must
     * yield the same CRC as the one-shot call. */
    uint32_t rolling = crc32c_extend(0, input, 4);
    rolling = crc32c_extend(rolling, input + 4, sizeof(input) - 4);
    assert(rolling == expected);

    printf("google_crc32c test OK: crc32c(\"123456789\") = 0x%08x\n", one_shot);
    return 0;
}
