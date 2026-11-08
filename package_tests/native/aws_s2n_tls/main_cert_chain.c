/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_s2n_tls: cert chain; calls s2n_init(), s2n_cert_chain_and_key_new(), s2n_cert_chain_and_key_load_pem(), s2n_cert_chain_and_key_free().
 */

#include <s2n.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(s2n_init() == S2N_SUCCESS);

    struct s2n_cert_chain_and_key *chain = s2n_cert_chain_and_key_new();
    assert(chain != NULL);

    /* Negative path: garbage PEM must fail the parser, NOT crash. The s2n
     * frontend forwards to libcrypto's PEM routines; if libcrypto isn't
     * linked correctly this often segfaults rather than returns an error. */
    const char *bad_cert_pem = "-----BEGIN CERTIFICATE-----\nNOTREALLYACERT\n-----END CERTIFICATE-----\n";
    const char *bad_key_pem = "-----BEGIN PRIVATE KEY-----\nNOTREALLYAKEY\n-----END PRIVATE KEY-----\n";
    int rc = s2n_cert_chain_and_key_load_pem(chain, bad_cert_pem, bad_key_pem);
    assert(rc != S2N_SUCCESS);
    /* After a failed load, errno must be populated. */
    assert(s2n_errno != 0);

    assert(s2n_cert_chain_and_key_free(chain) == S2N_SUCCESS);

    assert(s2n_cleanup() == S2N_SUCCESS);

    printf("aws_s2n_tls cert_chain test OK\n");
    return 0;
}
