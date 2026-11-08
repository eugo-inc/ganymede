/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ncurses: menu; calls newterm(), new_item(), item_name(), item_description().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ncurses.h>
#include <menu.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    FILE *null_in = fopen("/dev/null", "r");
    FILE *null_out = fopen("/dev/null", "w");
    assert(null_in != NULL && null_out != NULL);
    SCREEN *screen = newterm("xterm", null_out, null_in);
    assert(screen != NULL);

    /* Two items + NULL terminator — the menu API requires a NULL-terminated array. */
    ITEM *i1 = new_item("alpha", "first");
    assert(i1 != NULL);
    ITEM *i2 = new_item("beta", "second");
    assert(i2 != NULL);

    assert(strcmp(item_name(i1), "alpha") == 0);
    assert(strcmp(item_description(i1), "first") == 0);
    assert(strcmp(item_name(i2), "beta") == 0);

    ITEM *items[] = {i1, i2, NULL};
    MENU *m = new_menu(items);
    assert(m != NULL);

    /* item_count must report 2 non-NULL items. */
    assert(item_count(m) == 2);

    ITEM **back = menu_items(m);
    assert(back != NULL);
    assert(back[0] == i1);
    assert(back[1] == i2);

    int rc = free_menu(m);
    assert(rc == E_OK);
    rc = free_item(i1);
    assert(rc == E_OK);
    rc = free_item(i2);
    assert(rc == E_OK);

    /* Tear down the curses screen. */
    endwin();
    delscreen(screen);

    printf("ncurses menu test OK\n");
    return 0;
}
