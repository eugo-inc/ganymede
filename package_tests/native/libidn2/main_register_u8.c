/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libidn2: register u8; calls idn2_register_u8(), idn2_free().
 */

#include <idn2.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    uint8_t *insertname = NULL;
    int rc = idn2_register_u8(
        NULL,                                   /* ulabel: not supplied */
        (const uint8_t *)"xn--bcher-kva",       /* alabel: ACE form */
        &insertname,
        0                                       /* flags */
    );
    assert(rc == IDN2_OK);
    assert(insertname != NULL);

    assert(strcmp((const char *)insertname, "xn--bcher-kva") == 0);

    /* Result must be pure ASCII (an A-label always is). */
    for (const uint8_t *p = insertname; *p; ++p) {
        assert(*p < 0x80);
    }

    printf("libidn2 register_u8 OK: %s\n", (const char *)insertname);

    idn2_free(insertname);
    return 0;
}
