/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ncurses: window; calls newterm(), newwin(), getmaxyx(), mvwaddstr().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* ncurses needs a screen (SP) before newwin() will allocate a window;
     * without one, newwin() returns NULL. Bring one up against /dev/null so
     * no controlling TTY is required. */
    FILE *null_in = fopen("/dev/null", "r");
    FILE *null_out = fopen("/dev/null", "w");
    assert(null_in != NULL && null_out != NULL);
    SCREEN *screen = newterm("xterm", null_out, null_in);
    assert(screen != NULL);

    /* In-memory window: 10 rows x 40 cols, top-left at (0,0). */
    WINDOW *w = newwin(10, 40, 0, 0);
    assert(w != NULL);

    int rows = 0, cols = 0;
    getmaxyx(w, rows, cols);
    assert(rows == 10);
    assert(cols == 40);

    /* Write "hi" into the buffer at row 2, col 5. */
    int rc = mvwaddstr(w, 2, 5, "hi");
    assert(rc == OK);

    /* Read back the character at (2, 5). mvwinch returns a chtype with
     * attribute bits OR'd in; mask with A_CHARTEXT to get the bare char. */
    chtype ch = mvwinch(w, 2, 5);
    assert((ch & A_CHARTEXT) == (chtype)'h');

    /* Read back the second character at (2, 6) for good measure. */
    ch = mvwinch(w, 2, 6);
    assert((ch & A_CHARTEXT) == (chtype)'i');

    rc = wmove(w, 3, 7);
    assert(rc == OK);
    int y = -1, x = -1;
    getyx(w, y, x);
    assert(y == 3);
    assert(x == 7);

    /* Sub-window: 4 rows x 20 cols at (1, 2) relative to parent. */
    WINDOW *sub = derwin(w, 4, 20, 1, 2);
    assert(sub != NULL);
    int srows = 0, scols = 0;
    getmaxyx(sub, srows, scols);
    assert(srows == 4);
    assert(scols == 20);

    rc = delwin(sub);
    assert(rc == OK);
    rc = delwin(w);
    assert(rc == OK);

    /* Tear down the curses screen. */
    endwin();
    delscreen(screen);

    printf("ncurses window test OK\n");
    return 0;
}
