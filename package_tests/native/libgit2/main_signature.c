/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgit2: signature; calls git_libgit2_init(), git_signature_new(), git_signature_free(), git_signature_now().
 */

#include <git2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(git_libgit2_init() >= 1);

    git_signature *sig = NULL;
    const char *name = "Demo Tester";
    const char *email = "tester@example.org";
    git_time_t when = 1704164645;
    int offset_minutes = 0;

    assert(git_signature_new(&sig, name, email, when, offset_minutes) == 0);
    assert(sig != NULL);
    assert(strcmp(sig->name, name) == 0);
    assert(strcmp(sig->email, email) == 0);
    assert(sig->when.time == when);
    assert(sig->when.offset == offset_minutes);
    git_signature_free(sig);

    /* git_signature_now: must succeed and return a non-zero wall-clock time. */
    git_signature *sig_now = NULL;
    assert(git_signature_now(&sig_now, name, email) == 0);
    assert(sig_now != NULL);
    assert(sig_now->when.time > 0);
    assert(strcmp(sig_now->name, name) == 0);
    git_signature_free(sig_now);

    /* Empty name must be rejected (libgit2 validates inputs). */
    git_signature *bad = NULL;
    assert(git_signature_new(&bad, "", email, when, offset_minutes) != 0);
    assert(bad == NULL);

    assert(git_libgit2_shutdown() == 0);
    printf("libgit2 signature test OK\n");
    return 0;
}
