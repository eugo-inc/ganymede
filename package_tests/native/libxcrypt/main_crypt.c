/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxcrypt: crypt; calls crypt().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *setting = "$6$saltsalt";
    char *hash = crypt("hello", setting);
    assert(hash != NULL);
    /* libxcrypt's failure-token convention is a leading '*'. */
    assert(hash[0] != '*');
    assert(strncmp(hash, "$6$saltsalt$", 12) == 0);
    assert(strlen(hash) == 98);

    /* Capture the hash before the next crypt() call clobbers the static
     * buffer — this is the entire reason crypt_r/crypt_rn exist. */
    char captured[CRYPT_OUTPUT_SIZE];
    strncpy(captured, hash, sizeof(captured) - 1);
    captured[sizeof(captured) - 1] = '\0';

    /* Verify path: hashing the same password with the produced hash as the
     * setting reproduces the same string. This is the canonical legacy
     * password-verification idiom (getspnam + crypt). */
    char *again = crypt("hello", captured);
    assert(again != NULL);
    assert(again[0] != '*');
    assert(strcmp(again, captured) == 0);

    /* Negative path: a different password must NOT reproduce the hash. */
    char *bad = crypt("world", captured);
    assert(bad != NULL);
    assert(strcmp(bad, captured) != 0);

    /* Failure contract: crypt() with a garbage setting returns a
     * "*"-prefixed failure token (NOT NULL). This is the historical SYSV
     * contract and the documented behavior difference vs crypt_rn/crypt_ra
     * (both of which return NULL on bad input). */
    char *err = crypt("hello", "$NOPE$bogus");
    assert(err != NULL);
    assert(err[0] == '*');

    printf("libxcrypt crypt() test OK: %s\n", captured);
    return 0;
}
