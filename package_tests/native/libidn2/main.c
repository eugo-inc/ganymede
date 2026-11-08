/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libidn2: core API; calls idn2_to_ascii_8z().
 */

#include <idn2.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *ascii = NULL;
    /* "example.com" is ASCII already; the test is just that the function runs OK. */
    int rc = idn2_to_ascii_8z("example.com", &ascii, IDN2_NONTRANSITIONAL);
    assert(rc == IDN2_OK);
    assert(ascii != NULL);
    printf("libidn2 test OK: %s\n", ascii);
    free(ascii);
    return 0;
}
