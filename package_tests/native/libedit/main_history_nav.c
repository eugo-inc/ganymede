/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libedit: history nav; calls history_init(), history(), history_end().
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

    assert(history(h, &ev, H_SETSIZE, 50) == 0);

    /* Enter three events in insertion order. */
    assert(history(h, &ev, H_ENTER, "first")  >= 1);
    assert(history(h, &ev, H_ENTER, "second") >= 1);
    assert(history(h, &ev, H_ENTER, "third")  >= 1);

    /* H_LAST positions at the OLDEST event ("first" — the one entered earliest). */
    assert(history(h, &ev, H_LAST) == 0);
    assert(ev.str != NULL);
    assert(strcmp(ev.str, "first") == 0);

    /* H_PREV walks toward newer events (libedit history is a LIFO stack —
     * H_LAST is oldest, H_FIRST is newest; H_PREV moves toward H_FIRST). */
    assert(history(h, &ev, H_PREV) == 0);
    assert(strcmp(ev.str, "second") == 0);

    assert(history(h, &ev, H_PREV) == 0);
    assert(strcmp(ev.str, "third") == 0);

    /* H_CLEAR wipes all entries; a subsequent H_FIRST must fail (-1). */
    assert(history(h, &ev, H_CLEAR) == 0);
    assert(history(h, &ev, H_FIRST) == -1);

    history_end(h);

    printf("libedit history-nav test OK\n");
    return 0;
}
