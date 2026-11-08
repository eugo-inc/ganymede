/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmagic: params; calls magic_open(), magic_load(), magic_setparam(), magic_getparam().
 */

#include <magic.h>
#include <assert.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    magic_t cookie = magic_open(MAGIC_NONE);
    assert(cookie != NULL);
    int rc = magic_load(cookie, NULL);
    assert(rc == 0);

    size_t in_indir = 42;
    int sr = magic_setparam(cookie, MAGIC_PARAM_INDIR_MAX, &in_indir);
    assert(sr == 0);

    size_t out_indir = 0;
    int gr = magic_getparam(cookie, MAGIC_PARAM_INDIR_MAX, &out_indir);
    assert(gr == 0);
    assert(out_indir == 42);

    /* --- Assertion 2: independence of two different params. --- */
    size_t in_name = 137;
    sr = magic_setparam(cookie, MAGIC_PARAM_NAME_MAX, &in_name);
    assert(sr == 0);

    size_t out_name = 0;
    gr = magic_getparam(cookie, MAGIC_PARAM_NAME_MAX, &out_name);
    assert(gr == 0);
    assert(out_name == 137);

    /* INDIR_MAX must STILL be 42 — setting NAME_MAX must not have
     * clobbered it. Would FAIL if the param ABI mis-indexed an array. */
    size_t reread_indir = 0;
    gr = magic_getparam(cookie, MAGIC_PARAM_INDIR_MAX, &reread_indir);
    assert(gr == 0);
    assert(reread_indir == 42);

    /* --- Assertion 3: magic_list() returns 0 on success.
     * Point fd 1 at /dev/null while it runs so the listing does not
     * pollute test output, then restore the original stdout. This needs
     * no controlling terminal, so it also works under CI. */
    fflush(stdout);
    int saved_stdout = dup(STDOUT_FILENO);
    assert(saved_stdout >= 0);
    int devnull = open("/dev/null", O_WRONLY);
    assert(devnull >= 0);
    int redirected = dup2(devnull, STDOUT_FILENO);
    assert(redirected == STDOUT_FILENO);
    int lr = magic_list(cookie, NULL);
    fflush(stdout);
    int restored = dup2(saved_stdout, STDOUT_FILENO);
    assert(restored == STDOUT_FILENO);
    close(saved_stdout);
    close(devnull);
    assert(lr == 0);

    magic_close(cookie);
    printf("libmagic params/list test OK\n");
    return 0;
}
