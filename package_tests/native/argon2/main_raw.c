/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for argon2: raw; calls argon2d_hash_raw(), argon2id_hash_raw().
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

    /* Modest cost parameters — keep runtime well under the 10s budget. */
    const uint32_t t_cost = 2;
    const uint32_t m_cost = 1 << 12;  /* 4 MiB */
    const uint32_t parallelism = 1;
    const size_t hashlen = 32;

    uint8_t out_d[32]  = {0};
    uint8_t out_id[32] = {0};

    int rc = argon2d_hash_raw(
        t_cost, m_cost, parallelism,
        pwd, pwdlen,
        salt, sizeof(salt),
        out_d, hashlen
    );
    assert(rc == ARGON2_OK);

    rc = argon2id_hash_raw(
        t_cost, m_cost, parallelism,
        pwd, pwdlen,
        salt, sizeof(salt),
        out_id, hashlen
    );
    assert(rc == ARGON2_OK);

    /* Output buffer must have been written (not left all-zero). */
    int any_nonzero_d = 0, any_nonzero_id = 0;
    for (size_t i = 0; i < hashlen; ++i) {
        if (out_d[i]  != 0) any_nonzero_d  = 1;
        if (out_id[i] != 0) any_nonzero_id = 1;
    }
    assert(any_nonzero_d);
    assert(any_nonzero_id);

    /* argon2d and argon2id MUST produce different digests on the same input —
       if these collide the variant dispatch is broken. */
    assert(memcmp(out_d, out_id, hashlen) != 0);

    printf("argon2 raw test OK: argon2d[0..3]=%02x%02x%02x%02x argon2id[0..3]=%02x%02x%02x%02x\n",
           out_d[0], out_d[1], out_d[2], out_d[3],
           out_id[0], out_id[1], out_id[2], out_id[3]);
    return 0;
}
