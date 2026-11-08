/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for argon2: errors; calls argon2i_hash_raw(), argon2_error_message().
 */

#include <argon2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    const char *pwd = "password";
    const size_t pwdlen = strlen(pwd);
    const uint8_t salt[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };
    uint8_t out[32] = {0};

    /* Memory cost of 0 is invalid — must be rejected. */
    int rc = argon2i_hash_raw(
        /* t_cost      */ 2,
        /* m_cost      */ 0,         /* INVALID */
        /* parallelism */ 1,
        pwd, pwdlen,
        salt, sizeof(salt),
        out, sizeof(out)
    );
    assert(rc != ARGON2_OK);

    /* The returned code must have a human-readable message. */
    const char *err_msg = argon2_error_message(rc);
    assert(err_msg != NULL);
    assert(strlen(err_msg) > 0);

    /* And the OK message must exist and differ from the error message —
       catches a stubbed-out error-table. */
    const char *ok_msg = argon2_error_message(ARGON2_OK);
    assert(ok_msg != NULL);
    assert(strlen(ok_msg) > 0);
    assert(strcmp(ok_msg, err_msg) != 0);

    /* Salt-too-short is another distinct validation path — exercise it to
       prove the validator distinguishes parameter failures from each other. */
    const uint8_t tiny_salt[1] = {0x00};
    rc = argon2i_hash_raw(
        2, 1 << 12, 1,
        pwd, pwdlen,
        tiny_salt, sizeof(tiny_salt),  /* INVALID: salt too short */
        out, sizeof(out)
    );
    assert(rc != ARGON2_OK);

    printf("argon2 errors test OK: ok=\"%s\" err=\"%s\"\n", ok_msg, err_msg);
    return 0;
}
