/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ncurses: terminfo strings; calls setupterm(), tigetstr(), tparm().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <term.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int err = 0;
    int rc = setupterm("vt100", 1 /* fd: stdout, unused for cap lookup */, &err);
    assert(rc == OK);

    /* "cup" = cursor_address, a parameterized string capability. tigetstr returns
     * (char *)-1 if the name is not a string capability and NULL if it is absent
     * or cancelled. vt100 defines cup. */
    char *cup = tigetstr("cup");
    assert(cup != NULL);
    assert(cup != (char *)-1);

    char *expanded = tparm(cup, 5, 10);
    assert(expanded != NULL);
    assert(strchr(expanded, '\x1b') != NULL);

    /* "bel" = bell, a non-parameterized string cap. vt100 defines it as "\x07". */
    char *bel = tigetstr("bel");
    assert(bel != NULL);
    assert(bel != (char *)-1);
    assert(strlen(bel) > 0);

    printf("ncurses terminfo strings test OK: cup expanded to %zu bytes\n",
           strlen(expanded));
    return 0;
}
