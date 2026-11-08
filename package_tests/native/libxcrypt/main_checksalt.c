/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxcrypt: checksalt; calls crypt_checksalt(), crypt_r(), crypt_preferred_method().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
#if !defined(CRYPT_CHECKSALT_AVAILABLE) || CRYPT_CHECKSALT_AVAILABLE != 1
#  error "libxcrypt <crypt.h> not in include path (CRYPT_CHECKSALT_AVAILABLE)"
#endif
#if !defined(CRYPT_PREFERRED_METHOD_AVAILABLE) || CRYPT_PREFERRED_METHOD_AVAILABLE != 1
#  error "libxcrypt <crypt.h> not in include path (CRYPT_PREFERRED_METHOD_AVAILABLE)"
#endif

    assert(crypt_checksalt("$6$")  == CRYPT_SALT_OK);  /* sha512crypt */
    assert(crypt_checksalt("$y$")  == CRYPT_SALT_OK);  /* yescrypt */
    assert(crypt_checksalt("$2b$") == CRYPT_SALT_OK);  /* bcrypt */

    /* An empty setting is INVALID (no method can be recognized). */
    assert(crypt_checksalt("")     == CRYPT_SALT_INVALID);

    /* A well-formed two-character DES setting is never OK: DES is obsolete.
     * libxcrypt reports it as CRYPT_SALT_METHOD_LEGACY when the DES hashes
     * (descrypt/bigcrypt) are available, and as CRYPT_SALT_INVALID when
     * they are not. Accept either. */
    int des = crypt_checksalt("MN");
    assert(des == CRYPT_SALT_METHOD_LEGACY || des == CRYPT_SALT_INVALID);

    /* Outright garbage prefixes name no known method → INVALID. */
    assert(crypt_checksalt("$XX$") == CRYPT_SALT_INVALID);

    /* A full hashed-password string (not just the prefix) is also a valid
     * setting — this is how crypt_r's verify path works. */
    struct crypt_data data;
    memset(&data, 0, sizeof(data));
    const char *full = crypt_r("hello", "$6$saltsalt", &data);
    assert(full != NULL);
    assert(full[0] != '*');
    assert(crypt_checksalt(full) == CRYPT_SALT_OK);

    /* crypt_preferred_method: yescrypt is STRONG,DEFAULT in hashes.conf,
     * so the library must report "$y$". */
    const char *preferred = crypt_preferred_method();
    assert(preferred != NULL);
    assert(strcmp(preferred, "$y$") == 0);

    printf("libxcrypt checksalt test OK: preferred=%s\n", preferred);
    return 0;
}
