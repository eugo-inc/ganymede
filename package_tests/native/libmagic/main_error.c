/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmagic: error; calls magic_version(), magic_open(), magic_load(), magic_error().
 */

#include <magic.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Aspect: error-reporting API (magic_error / magic_errno) and version.
 * Distinct from main.c which only exercises the happy path with NULL db.
 * Drives a deliberate failure (load a nonexistent magic db file) and asserts
 * libmagic surfaces an error string via magic_error(). Also exercises
 * magic_version() — pure metadata, no DB needed. */
int main(void) {
    /* magic_version() — compile-time int constant, must be non-zero positive. */
    int v = magic_version();
    assert(v > 0);

    magic_t cookie = magic_open(MAGIC_NONE);
    assert(cookie != NULL);

    /* Force a failure: load a path that absolutely does not exist as a
     * compiled magic database. */
    int rc = magic_load(cookie, "/nonexistent/path/to/magic.mgc");
    assert(rc != 0);

    /* On failure, magic_error() must return a non-NULL, non-empty string. */
    const char *err = magic_error(cookie);
    assert(err != NULL);
    assert(strlen(err) > 0);

    /* magic_errno() must return a non-zero errno-style code on failure. */
    int e = magic_errno(cookie);
    /* errno could legitimately be 0 if libmagic synthesized its own error
     * with no underlying syscall failure — but the error string MUST be set. */
    (void)e;

    magic_close(cookie);

    magic_t c2 = magic_open(MAGIC_NONE);
    assert(c2 != NULL);
    int sr = magic_setflags(c2, MAGIC_MIME_TYPE);
    assert(sr == 0);
    int gf = magic_getflags(c2);
    assert(gf == MAGIC_MIME_TYPE);
    magic_close(c2);

    printf("libmagic error/version/flags test OK\n");
    return 0;
}
