/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for librhash: core API; calls rhash_msg(), rhash_print_bytes(), rhash_get_digest_size(), rhash_library_init().
 */

#include <rhash.h>
#include <stdio.h>
#include <string.h>

static int check(unsigned hash_id, const char *name, const char *expected) {
    unsigned char digest[64];
    char hex[130];
    if (rhash_msg(hash_id, "abc", 3, digest) < 0) {
        fprintf(stderr, "%s: rhash_msg failed\n", name);
        return 1;
    }
    rhash_print_bytes(hex, digest, rhash_get_digest_size(hash_id), RHPR_HEX);
    if (strcmp(hex, expected) != 0) {
        fprintf(stderr, "%s: got %s, expected %s\n", name, hex, expected);
        return 1;
    }
    return 0;
}

int main(void) {
    rhash_library_init();
    int failures = 0;
    failures += check(RHASH_SHA256, "SHA-256", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    failures += check(RHASH_SHA1, "SHA-1", "a9993e364706816aba3e25717850c26c9cd0d89d");
    failures += check(RHASH_MD5, "MD5", "900150983cd24fb0d6963f7d28e17f72");
    if (failures == 0) {
        printf("librhash OK\n");
    }
    return failures;
}
