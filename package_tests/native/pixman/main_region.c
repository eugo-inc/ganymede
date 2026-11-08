/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pixman: region; calls pixman_region32_init_rect(), pixman_region32_init(), pixman_region32_union(), pixman_region32_extents().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    pixman_region32_t a, b, result;

    /* a = [0,0,10,10], b = [5,5,15,15] — overlap region [5,5,10,10]. */
    pixman_region32_init_rect(&a, 0, 0, 10, 10);
    pixman_region32_init_rect(&b, 5, 5, 10, 10);
    pixman_region32_init(&result);

    /* Union: bounding extents must cover both inputs. */
    pixman_bool_t ok = pixman_region32_union(&result, &a, &b);
    assert(ok);
    pixman_box32_t *ext = pixman_region32_extents(&result);
    assert(ext != NULL);
    assert(ext->x1 == 0  && ext->y1 == 0);
    assert(ext->x2 == 15 && ext->y2 == 15);

    /* Intersection: must be the overlap rectangle exactly. */
    ok = pixman_region32_intersect(&result, &a, &b);
    assert(ok);
    ext = pixman_region32_extents(&result);
    assert(ext->x1 == 5  && ext->y1 == 5);
    assert(ext->x2 == 10 && ext->y2 == 10);

    /* Containment: (7,7) is inside the intersection, (0,0) is not. */
    assert(pixman_region32_contains_point(&result, 7, 7, NULL));
    assert(!pixman_region32_contains_point(&result, 0, 0, NULL));

    /* The intersection rectangle should not be empty. */
    assert(!pixman_region32_empty(&result));

    pixman_region32_fini(&a);
    pixman_region32_fini(&b);
    pixman_region32_fini(&result);

    printf("pixman region test OK\n");
    return 0;
}
