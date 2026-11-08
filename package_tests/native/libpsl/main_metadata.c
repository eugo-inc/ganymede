/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpsl: metadata; calls psl_builtin(), psl_suffix_count(), psl_suffix_exception_count(), psl_suffix_wildcard_count().
 */

#include <libpsl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const psl_ctx_t *psl = psl_builtin();
    assert(psl != NULL);

    /* The real publicsuffix.org list has thousands of suffixes, dozens of exceptions,
     * and dozens of wildcards. Asserting non-trivial lower bounds catches a busted blob. */
    int n_suffix = psl_suffix_count(psl);
    int n_exception = psl_suffix_exception_count(psl);
    int n_wildcard = psl_suffix_wildcard_count(psl);
    assert(n_suffix > 1000);        /* PSL has > 8000 entries in practice */
    assert(n_exception > 0);        /* a handful of !-rules */
    assert(n_wildcard > 0);         /* a handful of *.foo rules */

    const char *fname = psl_builtin_filename();
    assert(fname != NULL);
    assert(strlen(fname) > 0);

    const char *sha = psl_builtin_sha1sum();
    assert(sha != NULL);
    if (sha[0] != '\0') {
        assert(strlen(sha) == 40);
    }

    /* psl_check_version_number(0) is documented to return the runtime PSL_VERSION_NUMBER. */
    int runtime_ver = psl_check_version_number(0);
    assert(runtime_ver > 0);
    assert(runtime_ver == PSL_VERSION_NUMBER);

    const char *dist = psl_dist_filename();
    assert(dist != NULL);

    /* psl_builtin_outdated() returns 0 or 1 — must not crash. */
    int outdated = psl_builtin_outdated();
    assert(outdated == 0 || outdated == 1);

    printf("libpsl metadata test OK: suffixes=%d exceptions=%d wildcards=%d\n",
           n_suffix, n_exception, n_wildcard);
    return 0;
}
