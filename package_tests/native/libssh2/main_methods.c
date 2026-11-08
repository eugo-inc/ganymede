/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: methods; calls libssh2_init(), libssh2_session_init(), libssh2_session_supported_algs(), libssh2_free().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libssh2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int has_entry(const char **algs, int count, const char *needle) {
    for (int i = 0; i < count; i++) {
        if (algs[i] != NULL && strcmp(algs[i], needle) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(void) {
    assert(libssh2_init(0) == 0);

    LIBSSH2_SESSION *session = libssh2_session_init();
    assert(session != NULL);

    /* Query supported KEX algorithms. Any crypto backend registers at least one KEX algorithm. */
    const char **kex = NULL;
    int n_kex = libssh2_session_supported_algs(session, LIBSSH2_METHOD_KEX, &kex);
    assert(n_kex > 0);
    assert(kex != NULL);
    assert(has_entry(kex, n_kex, "curve25519-sha256")
           || has_entry(kex, n_kex, "diffie-hellman-group14-sha256")
           || has_entry(kex, n_kex, "diffie-hellman-group-exchange-sha256"));
    libssh2_free(session, kex);

    /* Query supported ciphers (client→server direction). */
    const char **ciphers = NULL;
    int n_ciphers = libssh2_session_supported_algs(session, LIBSSH2_METHOD_CRYPT_CS, &ciphers);
    assert(n_ciphers > 0);
    assert(ciphers != NULL);
    /* Every crypto backend provides the AES-CTR family. */
    assert(has_entry(ciphers, n_ciphers, "aes128-ctr")
           || has_entry(ciphers, n_ciphers, "aes256-ctr"));
    libssh2_free(session, ciphers);

    /* Query supported MACs. */
    const char **macs = NULL;
    int n_macs = libssh2_session_supported_algs(session, LIBSSH2_METHOD_MAC_CS, &macs);
    assert(n_macs > 0);
    libssh2_free(session, macs);

    int pref_rc = libssh2_session_method_pref(session, LIBSSH2_METHOD_CRYPT_CS, "aes128-ctr");
    assert(pref_rc == 0);

    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 methods test OK (kex=%d ciphers=%d macs=%d)\n", n_kex, n_ciphers, n_macs);
    return 0;
}
