/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libidn2: lookup u8; calls idn2_lookup_u8().
 */

#include <idn2.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const unsigned char input[] = {
        'b', 0xC3, 0xBC, 'c', 'h', 'e', 'r', '.', 'd', 'e', 0x00
    };

    uint8_t *ace = NULL;
    int rc = idn2_lookup_u8(input, &ace, IDN2_NONTRANSITIONAL);
    assert(rc == IDN2_OK);
    assert(ace != NULL);
    assert(strcmp((const char *)ace, "xn--bcher-kva.de") == 0);

    /* Result must be pure ASCII — every byte < 0x80. */
    for (const uint8_t *p = ace; *p; ++p) {
        assert(*p < 0x80);
    }

    printf("libidn2 lookup_u8 OK: %s\n", (const char *)ace);
    free(ace);
    return 0;
}
