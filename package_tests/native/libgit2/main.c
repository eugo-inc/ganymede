/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libgit2: core API; calls git_libgit2_init(), git_libgit2_version(), git_libgit2_features(), git_libgit2_shutdown().
 */

#include <git2.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* init returns the number of initializations of the library (>=1 on success). */
    int refcount = git_libgit2_init();
    assert(refcount >= 1);

    int major = 0, minor = 0, rev = 0;
    assert(git_libgit2_version(&major, &minor, &rev) == 0);
    assert(major >= 1);

    int features = git_libgit2_features();
    assert((features & GIT_FEATURE_THREADS) != 0);

    assert(git_libgit2_shutdown() == 0);

    printf("libgit2 test OK: %d.%d.%d\n", major, minor, rev);
    return 0;
}
