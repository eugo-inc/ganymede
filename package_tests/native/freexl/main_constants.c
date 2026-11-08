/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freexl: constants; compiles against <freexl.h>.
 */

#include <freexl.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Success sentinel. */
    assert(FREEXL_OK == 0);

    /* Cell-type tag values (see freexl 2.0 doxy docs). */
    assert(FREEXL_CELL_NULL   == 101);
    assert(FREEXL_CELL_INT    == 102);
    assert(FREEXL_CELL_DOUBLE == 103);
    assert(FREEXL_CELL_TEXT   == 104);
    assert(FREEXL_CELL_DATE   == 106);

    /* The five tag values must be mutually distinct. */
    assert(FREEXL_CELL_NULL   != FREEXL_CELL_INT);
    assert(FREEXL_CELL_INT    != FREEXL_CELL_DOUBLE);
    assert(FREEXL_CELL_DOUBLE != FREEXL_CELL_TEXT);
    assert(FREEXL_CELL_TEXT   != FREEXL_CELL_DATE);

    /* BIFF version tags. */
    assert(FREEXL_BIFF_VER_2 == 2);
    assert(FREEXL_BIFF_VER_3 == 3);
    assert(FREEXL_BIFF_VER_4 == 4);
    assert(FREEXL_BIFF_VER_5 == 5);
    assert(FREEXL_BIFF_VER_8 == 8);

    assert(FREEXL_FILE_NOT_FOUND < 0);

    printf("freexl constants test OK\n");
    return 0;
}
