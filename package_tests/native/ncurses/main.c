/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ncurses: core API; calls curses_version(), unctrl(), setupterm(), tigetnum().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <term.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version string must be present + identify itself as ncurses. */
    const char *v = curses_version();
    assert(v != NULL);
    assert(strstr(v, "ncurses") != NULL);

    /* unctrl() is a pure function — it maps a chtype to its printable form.
     * Control-A (0x01) prints as "^A". This exercises the lib without needing a TTY. */
    const char *ctrl_a = unctrl((chtype)0x01);
    assert(ctrl_a != NULL);
    assert(strcmp(ctrl_a, "^A") == 0);

    int err = 0;
    int rc = setupterm("dumb", 1 /* fd: stdout, unused for capability lookup */, &err);
    assert(rc == OK);

    int cols = tigetnum("cols");
    assert(cols != -2);

    printf("ncurses test OK: %s\n", v);
    return 0;
}
