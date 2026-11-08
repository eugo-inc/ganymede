/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for popt: bitset; calls poptGetContext(), poptGetNextOpt(), poptFreeContext().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* --- Part 1: POPT_ARG_VAL — assign opt->val directly into bound storage. --- */
    {
        int mode = 0;
        struct poptOption options[] = {
            { "fast",   'f', POPT_ARG_VAL, &mode, 1, "fast mode",   NULL },
            { "slow",   's', POPT_ARG_VAL, &mode, 2, "slow mode",   NULL },
            { "stable", 'S', POPT_ARG_VAL, &mode, 3, "stable mode", NULL },
            POPT_TABLEEND,
        };

        const char *argv[] = { "popt-val", "--slow", NULL };
        int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
        poptContext ctx = poptGetContext("popt-val", argc, argv, options, 0);
        assert(ctx != NULL);

        int rc;
        while ((rc = poptGetNextOpt(ctx)) >= 0) {
            /* val=2 is copied into mode by popt. */
        }
        assert(rc == -1);
        assert(mode == 2);
        poptFreeContext(ctx);
    }

    /* --- Part 2: POPT_BIT_SET — OR val into bound storage; cumulative across flags. --- */
    {
        int flags = 0;
        struct poptOption options[] = {
            { "read",  'r', POPT_BIT_SET, &flags, 0x1, "read bit",  NULL },
            { "write", 'w', POPT_BIT_SET, &flags, 0x2, "write bit", NULL },
            { "exec",  'x', POPT_BIT_SET, &flags, 0x4, "exec bit",  NULL },
            POPT_TABLEEND,
        };

        const char *argv[] = { "popt-bit", "-r", "-x", NULL };
        int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
        poptContext ctx = poptGetContext("popt-bit", argc, argv, options, 0);
        assert(ctx != NULL);

        int rc;
        while ((rc = poptGetNextOpt(ctx)) >= 0) {
            /* Each match OR's its val into flags. */
        }
        assert(rc == -1);
        /* -r sets 0x1, -x sets 0x4, -w was not passed. */
        assert(flags == (0x1 | 0x4));
        assert((flags & 0x2) == 0);
        poptFreeContext(ctx);
    }

    /* --- Part 3: POPT_BIT_CLR — clear bits that POPT_BIT_SET turned on. --- */
    {
        int flags = 0x7;  /* start with all three bits set */
        struct poptOption options[] = {
            { "clear-write", 'W', POPT_BIT_CLR, &flags, 0x2, "clear write", NULL },
            POPT_TABLEEND,
        };

        const char *argv[] = { "popt-clr", "--clear-write", NULL };
        int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;
        poptContext ctx = poptGetContext("popt-clr", argc, argv, options, 0);
        assert(ctx != NULL);

        int rc;
        while ((rc = poptGetNextOpt(ctx)) >= 0) {
            /* BIT_CLR clears the named bits. */
        }
        assert(rc == -1);
        assert((flags & 0x2) == 0);   /* write bit cleared */
        assert((flags & 0x1) == 0x1); /* read bit untouched */
        assert((flags & 0x4) == 0x4); /* exec bit untouched */
        poptFreeContext(ctx);
    }

    printf("popt bitset test OK\n");
    return 0;
}
