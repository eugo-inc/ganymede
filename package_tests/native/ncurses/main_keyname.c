/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ncurses: keyname; calls keyname(), setupterm(), tigetflag().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <term.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *f1 = keyname(KEY_F(1));
    assert(f1 != NULL);
    assert(strncmp(f1, "KEY_F", 5) == 0);

    /* Printable ASCII passes through as a single-char string. */
    const char *a = keyname((int)'A');
    assert(a != NULL);
    assert(strcmp(a, "A") == 0);

    /* Control char maps to caret notation, same as unctrl(). */
    const char *ctrl_a = keyname(0x01);
    assert(ctrl_a != NULL);
    assert(strcmp(ctrl_a, "^A") == 0);

    /* Now the boolean-capability leg. Load vt100. */
    int err = 0;
    int rc = setupterm("vt100", 1, &err);
    assert(rc == OK);

    /* tigetflag returns -1 if the name is not a boolean capability, 0 if it is
     * absent or cancelled, 1 if set. vt100 sets "am" (auto_right_margin). */
    int am = tigetflag("am");
    assert(am == 1);

    /* "xenl" must be a known boolean capability (0 or 1, never -1). */
    int xenl = tigetflag("xenl");
    assert(xenl == 0 || xenl == 1);

    printf("ncurses keyname/tigetflag test OK\n");
    return 0;
}
