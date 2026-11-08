/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ncurses: panel; calls newterm(), newpad(), new_panel(), panel_window().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <panel.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *in = fopen("/dev/null", "r");
    FILE *out = fopen("/dev/null", "w");
    assert(in != NULL);
    assert(out != NULL);

    SCREEN *screen = newterm("vt100", out, in);
    assert(screen != NULL);

    /* newpad creates an in-memory WINDOW (24x80); it needs the screen that newterm set up above. */
    WINDOW *pad = newpad(24, 80);
    assert(pad != NULL);

    PANEL *p = new_panel(pad);
    assert(p != NULL);

    WINDOW *w = panel_window(p);
    assert(w == pad);

    /* Teardown. del_panel must NOT free the underlying window — caller owns it. */
    int rc = del_panel(p);
    assert(rc == OK);

    rc = delwin(pad);
    assert(rc == OK);

    /* Tear down the screen + close the FILE handles. endwin() before
     * delscreen() per the ncurses man page. */
    endwin();
    delscreen(screen);
    fclose(in);
    fclose(out);

    printf("ncurses panel test OK\n");
    return 0;
}
