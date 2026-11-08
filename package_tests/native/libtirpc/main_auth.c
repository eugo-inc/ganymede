/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtirpc: auth; calls authnone_create(), authunix_create_default(), auth_destroy().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/auth.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* AUTH_NONE: the trivial null authenticator. */
    AUTH *none = authnone_create();
    assert(none != NULL);
    assert(none->ah_cred.oa_flavor == AUTH_NONE);

    /* AUTH_UNIX (a.k.a. AUTH_SYS): unix-creds default builder picks up
     * uid/gid/hostname from the current process. */
    AUTH *unix_auth = authunix_create_default();
    assert(unix_auth != NULL);
    assert(unix_auth->ah_cred.oa_flavor == AUTH_UNIX ||
           unix_auth->ah_cred.oa_flavor == AUTH_SYS);
    /* The unix credential body must carry a non-empty machname + non-empty
     * marshalled credentials buffer — proves the builder populated them. */
    assert(unix_auth->ah_cred.oa_base != NULL);
    assert(unix_auth->ah_cred.oa_length > 0);

    /* Both must support the ops-table destroy dispatch without crashing. */
    auth_destroy(none);
    auth_destroy(unix_auth);

    printf("libtirpc auth test OK\n");
    return 0;
}
