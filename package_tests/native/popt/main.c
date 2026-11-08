/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: core API; calls poptGetContext(), poptGetNextOpt(), poptStrerror(), poptFreeContext().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *name = NULL;
    int count = 0;

    struct poptOption options[] = {
        { "name",  'n', POPT_ARG_STRING, &name,  0, "name",  "NAME"  },
        { "count", 'c', POPT_ARG_INT,    &count, 0, "count", "COUNT" },
        POPT_TABLEEND,
    };

    const char *argv[] = { "popt-smoke", "--name=alice", "-c", "42", NULL };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    poptContext ctx = poptGetContext("popt-smoke", argc, argv, options, 0);
    assert(ctx != NULL);

    int rc;
    while ((rc = poptGetNextOpt(ctx)) >= 0) {
        /* Bound storage is populated by popt itself; nothing to do per-option. */
    }
    assert(rc == -1);  /* -1 == end of options, anything else is a parse error */

    assert(name != NULL);
    assert(strcmp(name, "alice") == 0);
    assert(count == 42);

    /* poptStrerror must resolve a known error code to a non-empty diagnostic. */
    const char *err = poptStrerror(POPT_ERROR_NOARG);
    assert(err != NULL);
    assert(err[0] != '\0');

    poptFreeContext(ctx);

    printf("popt test OK\n");
    return 0;
}
