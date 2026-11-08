/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxcrypt: bcrypt; calls crypt_r(), crypt_checksalt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* $2b$ = bcrypt. Cost "04" = 2^4 = 16 rounds; the minimum that
     * libxcrypt accepts. Keeps the test fast (< 5 ms). The salt is
     * exactly 22 characters of bcrypt's variant of base64. */
    const char *setting = "$2b$04$abcdefghijklmnopqrstuv";

    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    const char *hash = crypt_r("hello", setting, &data);
    assert(hash != NULL);
    /* failure-token sentinel — must NOT be present. */
    assert(hash[0] != '*');
    assert(strncmp(hash, "$2b$04$", 7) == 0);
    assert(strncmp(hash + 7, "abcdefghijklmnopqrstu", 21) == 0);
    /* bcrypt MCF hashes are always exactly 60 characters. */
    assert(strlen(hash) == 60);

    /* Save before reusing struct crypt_data. */
    char produced[CRYPT_OUTPUT_SIZE];
    strncpy(produced, hash, sizeof(produced) - 1);
    produced[sizeof(produced) - 1] = '\0';

    struct crypt_data verify;
    memset(&verify, 0, sizeof(verify));
    const char *again = crypt_r("hello", produced, &verify);
    assert(again != NULL);
    assert(again[0] != '*');
    assert(strcmp(again, produced) == 0);

    /* Negative path: a different password must NOT reproduce the hash. */
    struct crypt_data wrong;
    memset(&wrong, 0, sizeof(wrong));
    const char *bad = crypt_r("world", produced, &wrong);
    assert(bad != NULL);
    assert(strcmp(bad, produced) != 0);

    /* The produced full hash must classify as a valid setting under
     * crypt_checksalt — this exercises the bcrypt branch of the
     * checksalt parser, which is a separate code path from the hash
     * dispatch above. */
    assert(crypt_checksalt(produced) == CRYPT_SALT_OK);

    printf("libxcrypt bcrypt test OK: %s\n", produced);
    return 0;
}
