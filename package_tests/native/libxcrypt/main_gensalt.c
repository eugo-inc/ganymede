/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxcrypt: gensalt; calls crypt_gensalt_rn(), crypt_r().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    char salt_buf[CRYPT_GENSALT_OUTPUT_SIZE];
    memset(salt_buf, 0, sizeof(salt_buf));

    /* 16 bytes of deterministic "random" data — the SHA-512 crypt prefix
     * accepts up to 16 bytes of entropy. */
    const char rbytes[16] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    };

    char *setting = crypt_gensalt_rn(
        "$6$", 0, rbytes, (int)sizeof(rbytes),
        salt_buf, (int)sizeof(salt_buf));
    assert(setting != NULL);
    assert(setting == salt_buf);
    assert(strncmp(setting, "$6$", 3) == 0);        /* prefix preserved */
    assert(strlen(setting) > 3);                    /* actual salt appended */

    /* Save the first result. */
    char first[CRYPT_GENSALT_OUTPUT_SIZE];
    strncpy(first, setting, sizeof(first) - 1);
    first[sizeof(first) - 1] = '\0';

    /* Determinism: same rbytes → same setting. */
    char salt_buf2[CRYPT_GENSALT_OUTPUT_SIZE];
    memset(salt_buf2, 0, sizeof(salt_buf2));
    char *setting2 = crypt_gensalt_rn(
        "$6$", 0, rbytes, (int)sizeof(rbytes),
        salt_buf2, (int)sizeof(salt_buf2));
    assert(setting2 != NULL);
    assert(strcmp(first, setting2) == 0);

    /* End-to-end: the produced setting must be usable as the SETTING arg
     * to crypt_r. This is the real-world use of the gensalt API. */
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    const char *hash = crypt_r("hello", first, &data);
    assert(hash != NULL);
    assert(hash[0] != '*');                         /* not a failure token */
    assert(strncmp(hash, first, strlen(first)) == 0); /* hash embeds setting */

    /* Error path: a garbage prefix returns NULL. */
    char err_buf[CRYPT_GENSALT_OUTPUT_SIZE];
    memset(err_buf, 0, sizeof(err_buf));
    char *err = crypt_gensalt_rn(
        "$NOPE$", 0, rbytes, (int)sizeof(rbytes),
        err_buf, (int)sizeof(err_buf));
    assert(err == NULL);

    printf("libxcrypt gensalt test OK: %s\n", first);
    return 0;
}
