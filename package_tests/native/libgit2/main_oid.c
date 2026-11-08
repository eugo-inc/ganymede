/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgit2: oid; calls git_libgit2_init(), git_oid_fromstr(), git_oid_cmp(), git_oid_equal().
 */

#include <git2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(git_libgit2_init() >= 1);

    /* A known-good SHA-1 hex string (40 chars). */
    const char *hex_a = "1234567890abcdef1234567890abcdef12345678";
    const char *hex_b = "fedcba0987654321fedcba0987654321fedcba09";

    git_oid oid_a, oid_b, oid_a_copy;

    /* Parse hex into a git_oid. */
    assert(git_oid_fromstr(&oid_a, hex_a) == 0);
    assert(git_oid_fromstr(&oid_b, hex_b) == 0);
    assert(git_oid_fromstr(&oid_a_copy, hex_a) == 0);

    /* Equal OIDs compare equal; unequal OIDs do not. */
    assert(git_oid_cmp(&oid_a, &oid_a_copy) == 0);
    assert(git_oid_cmp(&oid_a, &oid_b) != 0);
    assert(git_oid_equal(&oid_a, &oid_a_copy) != 0);
    assert(git_oid_equal(&oid_a, &oid_b) == 0);

    char out[GIT_OID_HEXSZ + 1] = {0};
    git_oid_tostr(out, sizeof(out), &oid_a);
    assert(strcmp(out, hex_a) == 0);

    /* Invalid hex must fail. */
    assert(git_oid_fromstr(&oid_a, "notavalidsha") != 0);

    assert(git_libgit2_shutdown() == 0);
    printf("libgit2 OID test OK: %s\n", out);
    return 0;
}
