/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for minizip_ng: path; calls mz_path_get_filename(), mz_path_remove_extension(), mz_path_remove_filename(), mz_path_has_slash().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_os.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    /* --- mz_path_get_filename: base name of a forward-slash path. */
    const char *filename = NULL;
    int32_t rc = mz_path_get_filename("a/b/c/hello.txt", &filename);
    assert(rc == MZ_OK);
    assert(filename != NULL);
    assert(strcmp(filename, "hello.txt") == 0);

    /* No separator => MZ_EXIST_ERROR (per mz_os.c contract). */
    const char *no_sep = NULL;
    rc = mz_path_get_filename("plain", &no_sep);
    assert(rc == MZ_EXIST_ERROR);
    assert(no_sep == NULL);

    /* --- mz_path_remove_extension: nukes trailing ".ext". */
    char ext_buf[64];
    strcpy(ext_buf, "archive.tar.gz");
    rc = mz_path_remove_extension(ext_buf);
    assert(rc == MZ_OK);
    assert(strcmp(ext_buf, "archive.tar") == 0);

    /* --- mz_path_remove_filename: drops the last path component. */
    char dir_buf[64];
    strcpy(dir_buf, "a/b/c/hello.txt");
    rc = mz_path_remove_filename(dir_buf);
    assert(rc == MZ_OK);
    assert(strcmp(dir_buf, "a/b/c") == 0);

    /* --- mz_path_has_slash: MZ_OK iff path ends with a separator. */
    assert(mz_path_has_slash("a/b/") == MZ_OK);
    assert(mz_path_has_slash("a/b") == MZ_EXIST_ERROR);

    /* --- mz_path_convert_slashes: since minizip-ng 4.0.10, POSIX builds treat
     * only '/' as a separator, so backslashes may pass through unchanged;
     * older releases convert them too. Converting '/' to the requested slash
     * is applied on every version. */
    char conv_buf[32];
    strcpy(conv_buf, "a\\b\\c");
    rc = mz_path_convert_slashes(conv_buf, '/');
    assert(rc == MZ_OK);
    assert(strcmp(conv_buf, "a\\b\\c") == 0 || strcmp(conv_buf, "a/b/c") == 0);  /* either result is accepted */
    strcpy(conv_buf, "a/b/c");
    rc = mz_path_convert_slashes(conv_buf, '\\');
    assert(rc == MZ_OK);
    assert(strcmp(conv_buf, "a\\b\\c") == 0);  /* '/' -> '\' conversion works */

    /* --- mz_path_compare_wc: wildcard match. */
    assert(mz_path_compare_wc("hello.txt", "*.txt", 0) == MZ_OK);
    assert(mz_path_compare_wc("hello.bin", "*.txt", 0) == MZ_EXIST_ERROR);

    uint8_t rand_buf[16] = {0};
    rc = mz_os_rand(rand_buf, sizeof(rand_buf));
    /* mz_os_rand returns the number of bytes written, not MZ_OK. */
    assert(rc == (int32_t)sizeof(rand_buf));
    int any_nonzero = 0;
    for (size_t i = 0; i < sizeof(rand_buf); i++) {
        if (rand_buf[i] != 0) { any_nonzero = 1; break; }
    }
    assert(any_nonzero);

    printf("minizip_ng path helpers OK\n");
    return 0;
}
