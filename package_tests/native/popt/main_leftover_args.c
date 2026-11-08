/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: leftover args; calls poptGetContext(), poptGetNextOpt(), poptPeekArg(), poptGetArg().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int verbose = 0;

    struct poptOption options[] = {
        { "verbose", 'v', POPT_ARG_NONE, &verbose, 0, "verbose", NULL },
        POPT_TABLEEND,
    };

    /* Three positional args after the recognized -v option. */
    const char *argv[] = {
        "popt-leftover", "-v", "file1.txt", "file2.txt", "file3.txt", NULL
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    poptContext ctx = poptGetContext("popt-leftover", argc, argv, options, 0);
    assert(ctx != NULL);

    int rc;
    while ((rc = poptGetNextOpt(ctx)) >= 0) {
        /* Bound storage populated by popt. */
    }
    assert(rc == -1);
    assert(verbose == 1);

    const char *peek1 = poptPeekArg(ctx);
    const char *peek2 = poptPeekArg(ctx);
    assert(peek1 != NULL);
    assert(peek2 != NULL);
    assert(strcmp(peek1, "file1.txt") == 0);
    assert(strcmp(peek2, "file1.txt") == 0);

    /* poptGetArg consumes the first leftover. */
    const char *first = poptGetArg(ctx);
    assert(first != NULL);
    assert(strcmp(first, "file1.txt") == 0);

    /* poptGetArgs returns the residual tail as a NULL-terminated argv. */
    const char **rest = poptGetArgs(ctx);
    assert(rest != NULL);
    assert(rest[0] != NULL);
    assert(strcmp(rest[0], "file2.txt") == 0);
    assert(rest[1] != NULL);
    assert(strcmp(rest[1], "file3.txt") == 0);
    assert(rest[2] == NULL);

    poptFreeContext(ctx);

    printf("popt leftover-args test OK\n");
    return 0;
}
