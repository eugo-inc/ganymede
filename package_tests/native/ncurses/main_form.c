/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ncurses: form; calls newterm(), new_field(), set_field_buffer(), field_buffer().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <form.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Bring up a curses screen against /dev/null (no controlling TTY) so the
     * form sublibrary's field-buffer machinery is backed by valid screen
     * state. */
    FILE *null_in = fopen("/dev/null", "r");
    FILE *null_out = fopen("/dev/null", "w");
    assert(null_in != NULL && null_out != NULL);
    SCREEN *screen = newterm("xterm", null_out, null_in);
    assert(screen != NULL);

    /* new_field(rows, cols, frow, fcol, nrow, nbuf). 1x10 field at origin,
     * no off-screen rows, no extra buffers. */
    FIELD *f = new_field(1, 10, 0, 0, 0, 0);
    assert(f != NULL);

    /* Write a known string into buffer 0 (the visible text). */
    const char *src = "hello";
    int rc = set_field_buffer(f, 0, src);
    assert(rc == E_OK);

    char *out = field_buffer(f, 0);
    assert(out != NULL);
    assert(strncmp(out, src, strlen(src)) == 0);

    rc = free_field(f);
    assert(rc == E_OK);

    /* Tear down the curses screen. */
    endwin();
    delscreen(screen);

    printf("ncurses form test OK\n");
    return 0;
}
