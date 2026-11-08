/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxcrypt: rn yescrypt; calls crypt_rn().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* crypt_rn requires a buffer of at least sizeof(struct crypt_data). */
    struct crypt_data buf;
    memset(&buf, 0, sizeof(buf));

    /* $y$ = yescrypt. The cost parameter "j9T" is the lowest-supported
     * yescrypt cost — keeps the test fast (< 50 ms). The salt portion
     * "abcdefghijkl" is 12 chars of base64-alphabet data. */
    const char *setting = "$y$j9T$abcdefghijkl";
    char *hash = crypt_rn("hello", setting,
                          &buf, (int)sizeof(buf));
    assert(hash != NULL);
    /* crypt_rn writes the result into the supplied buffer (specifically
     * into the `output` field at offset 0 of struct crypt_data). */
    assert(hash == buf.output);
    assert(strncmp(hash, "$y$", 3) == 0);

    /* Save the produced hash to use as the verify-path setting. */
    char produced[CRYPT_OUTPUT_SIZE];
    strncpy(produced, hash, sizeof(produced) - 1);
    produced[sizeof(produced) - 1] = '\0';

    struct crypt_data buf2;
    memset(&buf2, 0, sizeof(buf2));
    char *again = crypt_rn("hello", produced,
                           &buf2, (int)sizeof(buf2));
    assert(again != NULL);
    assert(strcmp(again, produced) == 0);

    /* Negative path: wrong password must NOT reproduce the hash. */
    struct crypt_data buf3;
    memset(&buf3, 0, sizeof(buf3));
    char *bad = crypt_rn("world", produced,
                         &buf3, (int)sizeof(buf3));
    assert(bad != NULL);
    assert(strcmp(bad, produced) != 0);

    /* Failure mode: crypt_rn returns NULL on bad setting (not a "*"
     * failure token like crypt_r). This is the documented contract
     * difference between the two entry points. */
    struct crypt_data buf4;
    memset(&buf4, 0, sizeof(buf4));
    char *err = crypt_rn("hello", "$NOPE$bogus",
                         &buf4, (int)sizeof(buf4));
    assert(err == NULL);

    /* Failure mode: buffer too small → NULL. */
    struct crypt_data buf5;
    memset(&buf5, 0, sizeof(buf5));
    char *small = crypt_rn("hello", setting, &buf5, 16);
    assert(small == NULL);

    printf("libxcrypt rn+yescrypt test OK: %s\n", produced);
    return 0;
}
