/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_s2n_tls: connection; calls s2n_init(), s2n_connection_new(), s2n_connection_get_wire_bytes_in(), s2n_connection_get_wire_bytes_out().
 */

#include <s2n.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(s2n_init() == S2N_SUCCESS);

    /* SERVER-mode connection: the half of TLS that holds the cert + private key.
     * s2n_connection_new MUST return a non-NULL handle here; on failure the
     * library convention is to set s2n_errno and return NULL. */
    struct s2n_connection *server = s2n_connection_new(S2N_SERVER);
    assert(server != NULL);

    /* A fresh connection should report a sane, non-negative wire-bytes count. */
    uint64_t wire_in = s2n_connection_get_wire_bytes_in(server);
    uint64_t wire_out = s2n_connection_get_wire_bytes_out(server);
    assert(wire_in == 0);
    assert(wire_out == 0);

    /* CLIENT-mode is the symmetric path; must also allocate cleanly. */
    struct s2n_connection *client = s2n_connection_new(S2N_CLIENT);
    assert(client != NULL);

    /* Both handles must free without error. */
    assert(s2n_connection_free(server) == S2N_SUCCESS);
    assert(s2n_connection_free(client) == S2N_SUCCESS);

    assert(s2n_cleanup() == S2N_SUCCESS);

    printf("aws_s2n_tls connection test OK\n");
    return 0;
}
