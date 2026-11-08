/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: argv string; calls poptParseArgvString(), poptDupArgv().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* poptParseArgvString: respects single/double quotes and backslash escapes. */
    int argc = 0;
    const char **argv = NULL;
    int rc = poptParseArgvString("ls -la 'a b' \"c d\"", &argc, &argv);
    assert(rc == 0);
    assert(argv != NULL);
    assert(argc == 4);
    assert(strcmp(argv[0], "ls") == 0);
    assert(strcmp(argv[1], "-la") == 0);
    assert(strcmp(argv[2], "a b") == 0);
    assert(strcmp(argv[3], "c d") == 0);

    /* poptDupArgv: deep-copies an argv vector into one heap block. */
    int dup_argc = 0;
    const char **dup_argv = NULL;
    rc = poptDupArgv(argc, argv, &dup_argc, &dup_argv);
    assert(rc == 0);
    assert(dup_argc == argc);
    assert(dup_argv != NULL);
    for (int i = 0; i < argc; i++) {
        assert(dup_argv[i] != NULL);
        assert(strcmp(dup_argv[i], argv[i]) == 0);
    }

    /* Release the original parse buffer; the duplicate must still be valid. */
    free(argv);
    assert(strcmp(dup_argv[0], "ls") == 0);
    assert(strcmp(dup_argv[2], "a b") == 0);

    free(dup_argv);

    printf("popt argv-string test OK\n");
    return 0;
}
