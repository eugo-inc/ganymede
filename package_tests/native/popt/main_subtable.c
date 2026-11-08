/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: subtable; calls poptGetContext(), poptGetNextOpt(), poptFreeContext(), poptBadOption().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Sub-table: networking-related options. */
    const char *host = NULL;
    int port = 0;

    struct poptOption net_options[] = {
        { "host", 'H', POPT_ARG_STRING, &host, 0, "host", "HOST" },
        { "port", 'p', POPT_ARG_INT,    &port, 0, "port", "PORT" },
        POPT_TABLEEND,
    };

    /* Parent table: includes the sub-table plus its own option. */
    int verbose = 0;
    struct poptOption options[] = {
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, net_options, 0, "Network options", NULL },
        { "verbose", 'v', POPT_ARG_NONE, &verbose, 0, "verbose", NULL },
        POPT_TABLEEND,
    };

    const char *argv[] = {
        "popt-subtable", "--host=example.com", "--port", "8080", "-v", NULL
    };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    poptContext ctx = poptGetContext("popt-subtable", argc, argv, options, 0);
    assert(ctx != NULL);

    int rc;
    while ((rc = poptGetNextOpt(ctx)) >= 0) {
        /* Both tables' bindings populated by popt. */
    }
    assert(rc == -1);

    assert(host != NULL);
    assert(strcmp(host, "example.com") == 0);
    assert(port == 8080);
    assert(verbose == 1);

    poptFreeContext(ctx);

    /* Second parse: feed an unknown option and verify poptBadOption identifies it. */
    const char *bad_argv[] = { "popt-subtable", "--no-such-flag", NULL };
    int bad_argc = (int)(sizeof(bad_argv) / sizeof(bad_argv[0])) - 1;

    poptContext bad_ctx = poptGetContext("popt-subtable", bad_argc, bad_argv, options, 0);
    assert(bad_ctx != NULL);

    rc = poptGetNextOpt(bad_ctx);
    assert(rc < -1);  /* < -1 == parse error (not end-of-options) */
    assert(rc == POPT_ERROR_BADOPT);

    const char *bad = poptBadOption(bad_ctx, POPT_BADOPTION_NOALIAS);
    assert(bad != NULL);
    assert(strcmp(bad, "--no-such-flag") == 0);

    poptFreeContext(bad_ctx);

    printf("popt subtable test OK\n");
    return 0;
}
