/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libedit: core API; calls history_init(), history(), history_end().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <histedit.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    History *h = history_init();
    assert(h != NULL);

    HistEvent ev;
    /* Set the history size to 100 first — required before H_ENTER on most libedit versions. */
    assert(history(h, &ev, H_SETSIZE, 100) == 0);

    /* Enter one event; H_ENTER returns the new event number (>= 1) on success. */
    int entered = history(h, &ev, H_ENTER, "echo hello");
    assert(entered >= 1);

    assert(history(h, &ev, H_FIRST) == 0);
    assert(ev.str != NULL);
    assert(strcmp(ev.str, "echo hello") == 0);

    history_end(h);

    printf("libedit test OK\n");
    return 0;
}
