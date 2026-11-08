/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_s2n_tls: cipher prefs; calls s2n_init(), s2n_config_new(), s2n_config_set_cipher_preferences(), s2n_strerror().
 */

#include <s2n.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(s2n_init() == S2N_SUCCESS);

    struct s2n_config *config = s2n_config_new();
    assert(config != NULL);

    /* "default" is the canonical security policy and MUST be recognized. */
    assert(s2n_config_set_cipher_preferences(config, "default") == S2N_SUCCESS);

    assert(s2n_config_set_cipher_preferences(config, "20240501") == S2N_SUCCESS);

    /* Negative path: an obviously-invalid policy name must fail AND set
     * s2n_errno. Confirms error-reporting plumbing works end-to-end. */
    int rc = s2n_config_set_cipher_preferences(config, "this_policy_does_not_exist_xyz");
    assert(rc != S2N_SUCCESS);
    assert(s2n_errno != 0);

    const char *msg = s2n_strerror(s2n_errno, "EN");
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    assert(s2n_config_free(config) == S2N_SUCCESS);
    assert(s2n_cleanup() == S2N_SUCCESS);

    printf("aws_s2n_tls cipher_prefs test OK: err_msg=\"%s\"\n", msg);
    return 0;
}
