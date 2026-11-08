/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpsl: cookie; calls psl_builtin(), psl_is_cookie_domain_acceptable().
 */

#include <libpsl.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const psl_ctx_t *psl = psl_builtin();
    assert(psl != NULL);

    /* "www.example.com" with cookie Domain="example.com" — acceptable (cookie domain is
     * a parent of hostname AND example.com is registrable, not a public suffix). */
    assert(psl_is_cookie_domain_acceptable(psl, "www.example.com", "example.com") == 1);

    /* Same hostname/domain — acceptable (host-only cookie semantics). */
    assert(psl_is_cookie_domain_acceptable(psl, "example.com", "example.com") == 1);

    /* Hostname "www.example.com" with cookie Domain="other.com" — NOT acceptable
     * (other.com is not a parent of www.example.com). */
    assert(psl_is_cookie_domain_acceptable(psl, "www.example.com", "other.com") == 0);

    /* Hostname under "co.uk" with cookie Domain="co.uk" — NOT acceptable (co.uk is a
     * public suffix; allowing this would let one site cookie-poison every other .co.uk site). */
    assert(psl_is_cookie_domain_acceptable(psl, "foo.co.uk", "co.uk") == 0);

    assert(psl_is_cookie_domain_acceptable(psl, "foo.bar.co.uk", "bar.co.uk") == 1);

    printf("libpsl cookie-domain test OK\n");
    return 0;
}
