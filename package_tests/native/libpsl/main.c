/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpsl: core API; calls psl_builtin(), psl_is_public_suffix(), psl_registrable_domain(), psl_get_version().
 */

#include <libpsl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const psl_ctx_t *psl = psl_builtin();
    assert(psl != NULL);

    /* "com" is a top-level public suffix — must classify as public. */
    assert(psl_is_public_suffix(psl, "com") == 1);

    /* "example.com" is a registrable domain under "com" — must NOT be a public suffix. */
    assert(psl_is_public_suffix(psl, "example.com") == 0);

    /* Registrable domain (eTLD+1) of "www.example.com" is "example.com". */
    const char *registrable = psl_registrable_domain(psl, "www.example.com");
    assert(registrable != NULL);
    assert(strcmp(registrable, "example.com") == 0);

    printf("libpsl test OK: %s\n", psl_get_version());
    return 0;
}
