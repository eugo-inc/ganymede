/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pango: tab array; calls pango_tab_array_new(), pango_tab_array_get_size(), pango_tab_array_get_positions_in_pixels(), pango_tab_array_set_tab().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Construct a 3-stop tab array in Pango units. */
    PangoTabArray *tabs = pango_tab_array_new(3, FALSE);
    assert(tabs != NULL);
    assert(pango_tab_array_get_size(tabs) == 3);
    assert(pango_tab_array_get_positions_in_pixels(tabs) == FALSE);

    pango_tab_array_set_tab(tabs, 0, PANGO_TAB_LEFT, 100 * PANGO_SCALE);
    pango_tab_array_set_tab(tabs, 1, PANGO_TAB_LEFT, 200 * PANGO_SCALE);
    pango_tab_array_set_tab(tabs, 2, PANGO_TAB_LEFT, 300 * PANGO_SCALE);

    PangoTabAlign align = (PangoTabAlign)-1;
    int location = -1;
    pango_tab_array_get_tab(tabs, 0, &align, &location);
    assert(align == PANGO_TAB_LEFT);
    assert(location == 100 * PANGO_SCALE);

    pango_tab_array_get_tab(tabs, 2, &align, &location);
    assert(align == PANGO_TAB_LEFT);
    assert(location == 300 * PANGO_SCALE);

    /* Bulk read via pango_tab_array_get_tabs. */
    PangoTabAlign *aligns = NULL;
    int *locations = NULL;
    pango_tab_array_get_tabs(tabs, &aligns, &locations);
    assert(locations != NULL);
    assert(locations[0] == 100 * PANGO_SCALE);
    assert(locations[1] == 200 * PANGO_SCALE);
    assert(locations[2] == 300 * PANGO_SCALE);
    g_free(aligns);
    g_free(locations);

    /* Resize and confirm. */
    pango_tab_array_resize(tabs, 5);
    assert(pango_tab_array_get_size(tabs) == 5);

    /* Copy + verify independent. */
    PangoTabArray *copy = pango_tab_array_copy(tabs);
    assert(copy != NULL);
    assert(pango_tab_array_get_size(copy) == 5);

    pango_tab_array_free(copy);
    pango_tab_array_free(tabs);

    printf("pango tab_array test OK\n");
    return 0;
}
