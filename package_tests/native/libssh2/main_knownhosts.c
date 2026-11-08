/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libssh2: knownhosts; calls libssh2_init(), libssh2_session_init(), libssh2_knownhost_init(), libssh2_knownhost_addc().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libssh2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(libssh2_init(0) == 0);

    LIBSSH2_SESSION *session = libssh2_session_init();
    assert(session != NULL);

    LIBSSH2_KNOWNHOSTS *kh = libssh2_knownhost_init(session);
    assert(kh != NULL);

    /* A synthetic ssh-rsa public key blob — opaque bytes; libssh2 just hashes + stores them.
     * The host string "example.test" + this blob form the known-host entry. */
    static const unsigned char fake_key[] = {
        0x00, 0x00, 0x00, 0x07, 's',  's',  'h',  '-',  'r',  's',  'a',
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15
    };

    int rc = libssh2_knownhost_addc(
        kh,
        "example.test", NULL,
        (const char *)fake_key, sizeof(fake_key),
        NULL, 0,
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW | LIBSSH2_KNOWNHOST_KEY_SSHRSA,
        NULL);
    assert(rc == 0);

    /* Same host + same key bytes → must report MATCH. */
    int check = libssh2_knownhost_check(
        kh, "example.test",
        (const char *)fake_key, sizeof(fake_key),
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
        NULL);
    assert(check == LIBSSH2_KNOWNHOST_CHECK_MATCH);

    /* Same host + DIFFERENT key bytes → must report MISMATCH (the security-critical path). */
    unsigned char other_key[sizeof(fake_key)];
    memcpy(other_key, fake_key, sizeof(fake_key));
    other_key[sizeof(other_key) - 1] ^= 0xFF;
    int check_bad = libssh2_knownhost_check(
        kh, "example.test",
        (const char *)other_key, sizeof(other_key),
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
        NULL);
    assert(check_bad == LIBSSH2_KNOWNHOST_CHECK_MISMATCH);

    /* Unknown host → must report NOTFOUND. */
    int check_unknown = libssh2_knownhost_check(
        kh, "nonexistent.test",
        (const char *)fake_key, sizeof(fake_key),
        LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW,
        NULL);
    assert(check_unknown == LIBSSH2_KNOWNHOST_CHECK_NOTFOUND);

    libssh2_knownhost_free(kh);
    libssh2_session_free(session);
    libssh2_exit();

    printf("libssh2 knownhosts test OK\n");
    return 0;
}
