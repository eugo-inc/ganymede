/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_s2n_tls: core API; calls s2n_init(), s2n_config_new(), s2n_get_openssl_version(), s2n_config_free().
 */

#include <s2n.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* s2n_init wires up the openssl-backed crypto subsystem; failure here usually
     * means libcrypto symbols didn't resolve at runtime. */
    assert(s2n_init() == S2N_SUCCESS);

    /* A config object is the entry point to s2n's TLS API. */
    struct s2n_config *config = s2n_config_new();
    assert(config != NULL);

    /* libcrypto sanity: the linked openssl must report a non-zero version. */
    unsigned long crypto_ver = s2n_get_openssl_version();
    assert(crypto_ver != 0);

    assert(s2n_config_free(config) == S2N_SUCCESS);
    assert(s2n_cleanup() == S2N_SUCCESS);

    printf("aws_s2n_tls test OK: libcrypto=0x%lx\n", crypto_ver);
    return 0;
}
