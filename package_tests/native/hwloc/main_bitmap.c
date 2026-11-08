/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for hwloc: bitmap; calls hwloc_bitmap_alloc(), hwloc_bitmap_iszero(), hwloc_bitmap_weight(), hwloc_bitmap_set().
 */

#include <hwloc.h>
#include <hwloc/bitmap.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    hwloc_bitmap_t a = hwloc_bitmap_alloc();
    assert(a != NULL);
    assert(hwloc_bitmap_iszero(a));
    assert(hwloc_bitmap_weight(a) == 0);

    /* Set a handful of bits, verify membership + popcount. */
    hwloc_bitmap_set(a, 0);
    hwloc_bitmap_set(a, 3);
    hwloc_bitmap_set(a, 17);
    assert(hwloc_bitmap_isset(a, 0));
    assert(hwloc_bitmap_isset(a, 3));
    assert(hwloc_bitmap_isset(a, 17));
    assert(!hwloc_bitmap_isset(a, 1));
    assert(hwloc_bitmap_weight(a) == 3);
    assert(hwloc_bitmap_first(a) == 0);
    assert(hwloc_bitmap_last(a) == 17);

    /* dup + equal. */
    hwloc_bitmap_t b = hwloc_bitmap_dup(a);
    assert(b != NULL);
    assert(hwloc_bitmap_isequal(a, b));

    /* Clearing one bit breaks equality, restoring it heals it. */
    hwloc_bitmap_clr(b, 3);
    assert(!hwloc_bitmap_isequal(a, b));
    assert(hwloc_bitmap_weight(b) == 2);
    hwloc_bitmap_set(b, 3);
    assert(hwloc_bitmap_isequal(a, b));

    char buf[128];
    int n = hwloc_bitmap_list_snprintf(buf, sizeof(buf), a);
    assert(n > 0);
    assert((size_t)n < sizeof(buf));

    hwloc_bitmap_t c = hwloc_bitmap_alloc();
    assert(c != NULL);
    assert(hwloc_bitmap_list_sscanf(c, buf) == 0);
    assert(hwloc_bitmap_isequal(a, c));

    /* zero() resets back to empty; full bitmap weight is reported as -1 (infinite). */
    hwloc_bitmap_zero(c);
    assert(hwloc_bitmap_iszero(c));
    hwloc_bitmap_fill(c);
    assert(hwloc_bitmap_weight(c) == -1);

    hwloc_bitmap_free(a);
    hwloc_bitmap_free(b);
    hwloc_bitmap_free(c);

    printf("hwloc bitmap test OK: list-format='%s'\n", buf);
    return 0;
}
