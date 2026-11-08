/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for argon2: core API; calls argon2i_hash_encoded(), argon2_verify().
 */

#include <argon2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *pwd = "password";
    const size_t pwdlen = strlen(pwd);
    const uint8_t salt[16] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    };

    /* Modest cost parameters — keep runtime well under the 10s budget. */
    const uint32_t t_cost = 2;        /* iterations */
    const uint32_t m_cost = 1 << 12;  /* 4 MiB memory */
    const uint32_t parallelism = 1;
    const size_t hashlen = 32;

    char encoded[256];
    int rc = argon2i_hash_encoded(
        t_cost, m_cost, parallelism,
        pwd, pwdlen,
        salt, sizeof(salt),
        hashlen,
        encoded, sizeof(encoded)
    );
    assert(rc == ARGON2_OK);
    assert(strncmp(encoded, "$argon2i$", 9) == 0);

    rc = argon2_verify(encoded, pwd, pwdlen, Argon2_i);
    assert(rc == ARGON2_OK);

    /* Verify must FAIL on a wrong password (catches a broken verify primitive). */
    rc = argon2_verify(encoded, "wrong", 5, Argon2_i);
    assert(rc != ARGON2_OK);

    printf("argon2 test OK: %s\n", encoded);
    return 0;
}
