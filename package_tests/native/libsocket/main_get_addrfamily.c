/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsocket: get addrfamily; calls get_address_family().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libsocket/libinetsocket.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* IPv4 literal — must classify as LIBSOCKET_IPv4. */
    int fam_v4 = get_address_family("127.0.0.1");
    assert(fam_v4 == LIBSOCKET_IPv4);

    /* IPv6 literal — must classify as LIBSOCKET_IPv6. */
    int fam_v6 = get_address_family("::1");
    assert(fam_v6 == LIBSOCKET_IPv6);

    assert(fam_v4 != fam_v6);

    printf("libsocket get_address_family test OK\n");
    return 0;
}
