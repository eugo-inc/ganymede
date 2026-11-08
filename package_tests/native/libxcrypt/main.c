/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxcrypt: core API; calls crypt_r().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct crypt_data data;
    memset(&data, 0, sizeof(data));

    /* SHA-512 ($6$) with an explicit salt — output is fully deterministic. */
    const char *setting = "$6$saltsalt";
    const char *hash = crypt_r("hello", setting, &data);
    assert(hash != NULL);
    /* libxcrypt signals failure by returning a string starting with '*'. */
    assert(hash[0] != '*');
    assert(strncmp(hash, "$6$saltsalt$", 12) == 0);
    /* Total length is 12 (prefix+salt+'$') + 86 = 98. */
    assert(strlen(hash) == 98);

    /* Save the produced hash before the next crypt_r call. */
    char produced[CRYPT_OUTPUT_SIZE];
    strncpy(produced, hash, sizeof(produced) - 1);
    produced[sizeof(produced) - 1] = '\0';

    /* Verify path: hashing "hello" again with the produced hash as the setting
     * MUST reproduce the same string — this is how crypt(3)-style password
     * verification works. Use a separate crypt_data to avoid any aliasing. */
    struct crypt_data verify;
    memset(&verify, 0, sizeof(verify));
    const char *again = crypt_r("hello", produced, &verify);
    assert(again != NULL);
    assert(again[0] != '*');
    assert(strcmp(again, produced) == 0);

    /* Negative path: a different password must NOT produce the same hash. */
    struct crypt_data wrong;
    memset(&wrong, 0, sizeof(wrong));
    const char *bad = crypt_r("world", produced, &wrong);
    assert(bad != NULL);
    assert(strcmp(bad, produced) != 0);

    printf("libxcrypt test OK: %s\n", produced);
    return 0;
}
