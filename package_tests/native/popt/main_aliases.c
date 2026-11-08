/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: aliases; calls poptGetContext(), poptAddItem(), poptGetNextOpt(), poptFreeContext().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int a = 0, b = 0, c = 0;

    struct poptOption options[] = {
        { "alpha", 'a', POPT_ARG_NONE, &a, 0, "alpha", NULL },
        { "beta",  'b', POPT_ARG_NONE, &b, 0, "beta",  NULL },
        { "gamma", 'c', POPT_ARG_NONE, &c, 0, "gamma", NULL },
        POPT_TABLEEND,
    };

    const char *argv[] = { "popt-alias", "--all", NULL };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    poptContext ctx = poptGetContext("popt-alias", argc, argv, options, 0);
    assert(ctx != NULL);

    struct poptItem_s item = {0};
    item.option.longName = "all";
    item.option.shortName = '\0';
    item.option.argInfo = POPT_ARG_NONE;
    item.option.descrip = "enable everything";

    /* popt takes ownership of argv and frees it with one free(), so the strings must live in the
     * same allocation; poptParseArgvString builds exactly that. */
    int expansion_argc = 0;
    const char **expansion = NULL;
    int rc = poptParseArgvString("--alpha --beta --gamma", &expansion_argc, &expansion);
    assert(rc == 0 && expansion_argc == 3);
    assert(expansion != NULL && strcmp(expansion[2], "--gamma") == 0);

    item.argc = expansion_argc;
    item.argv = expansion;

    /* flags=0 means "alias" (1 would mean "exec"). */
    rc = poptAddItem(ctx, &item, 0);
    assert(rc == 0);

    while ((rc = poptGetNextOpt(ctx)) >= 0) {
        /* The alias triggers all three bindings. */
    }
    assert(rc == -1);

    /* All three underlying flags must have flipped from the single --all. */
    assert(a == 1);
    assert(b == 1);
    assert(c == 1);

    poptFreeContext(ctx);

    printf("popt alias test OK\n");
    return 0;
}
