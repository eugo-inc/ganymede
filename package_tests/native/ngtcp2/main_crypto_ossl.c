/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ngtcp2: crypto ossl; calls ngtcp2_crypto_ossl_init(), ngtcp2_crypto_ossl_free().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ngtcp2/ngtcp2_crypto_ossl.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const int rc = ngtcp2_crypto_ossl_init();
    assert(rc == 0);

    ngtcp2_crypto_ossl_free();

    printf("ngtcp2_crypto_ossl test OK: init/free round-trip (rc=%d)\n", rc);
    return 0;
}
