/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for popt: callback; calls poptGetContext(), poptGetNextOpt(), poptFreeContext().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <popt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

struct cb_state {
    int pre_count;
    int post_count;
    int option_count;
    char last_long[64];
};

static void my_cb(poptContext con,
                  enum poptCallbackReason reason,
                  const struct poptOption *opt,
                  const char *arg,
                  const void *data) {
    (void)con;
    (void)arg;
    struct cb_state *st = (struct cb_state *)data;
    assert(st != NULL);
    switch (reason) {
        case POPT_CALLBACK_REASON_PRE:
            st->pre_count++;
            break;
        case POPT_CALLBACK_REASON_POST:
            st->post_count++;
            break;
        case POPT_CALLBACK_REASON_OPTION:
            st->option_count++;
            assert(opt != NULL);
            assert(opt->longName != NULL);
            strncpy(st->last_long, opt->longName, sizeof(st->last_long) - 1);
            st->last_long[sizeof(st->last_long) - 1] = '\0';
            break;
    }
}

int main(void) {
    struct cb_state st = { 0, 0, 0, "" };

    struct poptOption options[] = {
        /* POPT_ARG_CALLBACK: arg = callback fn, descrip = opaque data ptr.
         * POPT_CBFLAG_PRE|POST asks popt to fire before/after the parse. */
        { NULL, '\0',
          POPT_ARG_CALLBACK | POPT_CBFLAG_PRE | POPT_CBFLAG_POST,
          (void *)my_cb, 0, (const char *)&st, NULL },
        { "alpha", 'a', POPT_ARG_NONE, NULL, 0, "alpha", NULL },
        { "beta",  'b', POPT_ARG_NONE, NULL, 0, "beta",  NULL },
        POPT_TABLEEND,
    };

    const char *argv[] = { "popt-cb", "--alpha", "--beta", NULL };
    int argc = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    poptContext ctx = poptGetContext("popt-cb", argc, argv, options, 0);
    assert(ctx != NULL);

    int rc;
    while ((rc = poptGetNextOpt(ctx)) >= 0) {
        /* The callback fires for each matched option. */
    }
    assert(rc == -1);

    /* PRE fires exactly once at the start, POST exactly once at the end. */
    assert(st.pre_count == 1);
    assert(st.post_count == 1);

    /* Two recognized options means two OPTION-reason callbacks. */
    assert(st.option_count == 2);

    /* Last matched option must be "beta" (parse order is left-to-right). */
    assert(strcmp(st.last_long, "beta") == 0);

    poptFreeContext(ctx);

    printf("popt callback test OK\n");
    return 0;
}
