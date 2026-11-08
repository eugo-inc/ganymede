/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for argon2: encodedlen; calls argon2_encodedlen(), argon2d_hash_encoded(), argon2id_hash_encoded().
 */

#include <argon2.h>
#include <assert.h>
#include <stdlib.h>
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
    const uint32_t t_cost = 2;
    const uint32_t m_cost = 1 << 12;
    const uint32_t parallelism = 1;
    const size_t hashlen = 32;
    const size_t saltlen = sizeof(salt);

    /* Sizing helper must return positive for each variant. */
    const size_t enc_i  = argon2_encodedlen(t_cost, m_cost, parallelism, saltlen, hashlen, Argon2_i);
    const size_t enc_d  = argon2_encodedlen(t_cost, m_cost, parallelism, saltlen, hashlen, Argon2_d);
    const size_t enc_id = argon2_encodedlen(t_cost, m_cost, parallelism, saltlen, hashlen, Argon2_id);
    assert(enc_i  > 0);
    assert(enc_d  > 0);
    assert(enc_id > 0);

    /* Allocate exactly what the sizing helper reports, then hash into it.
       An off-by-one in encodedlen would surface here as ARGON2_OUTPUT_TOO_SHORT. */
    char *buf_d = (char *)malloc(enc_d);
    char *buf_id = (char *)malloc(enc_id);
    assert(buf_d != NULL);
    assert(buf_id != NULL);

    int rc = argon2d_hash_encoded(
        t_cost, m_cost, parallelism,
        pwd, pwdlen,
        salt, saltlen,
        hashlen,
        buf_d, enc_d
    );
    assert(rc == ARGON2_OK);

    rc = argon2id_hash_encoded(
        t_cost, m_cost, parallelism,
        pwd, pwdlen,
        salt, saltlen,
        hashlen,
        buf_id, enc_id
    );
    assert(rc == ARGON2_OK);

    /* Each variant emits its own PHC-format prefix. */
    assert(strncmp(buf_d,  "$argon2d$",  9) == 0);
    assert(strncmp(buf_id, "$argon2id$", 10) == 0);

    assert(strcmp(buf_d, buf_id) != 0);

    printf("argon2 encodedlen test OK: lens i=%zu d=%zu id=%zu\n", enc_i, enc_d, enc_id);

    free(buf_d);
    free(buf_id);
    return 0;
}
