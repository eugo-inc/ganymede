/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pixman: transform; calls pixman_transform_init_identity(), pixman_int_to_fixed(), pixman_transform_init_translate(), pixman_transform_multiply().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    pixman_transform_t identity;
    pixman_transform_init_identity(&identity);

    assert(identity.matrix[0][0] == pixman_int_to_fixed(1));
    assert(identity.matrix[1][1] == pixman_int_to_fixed(1));
    assert(identity.matrix[2][2] == pixman_int_to_fixed(1));
    assert(identity.matrix[0][1] == 0);
    assert(identity.matrix[1][0] == 0);

    pixman_transform_t translate;
    pixman_transform_init_translate(
        &translate, pixman_int_to_fixed(3), pixman_int_to_fixed(5));

    /* identity * translate must equal translate. */
    pixman_transform_t product;
    pixman_bool_t ok = pixman_transform_multiply(&product, &identity, &translate);
    assert(ok);
    assert(product.matrix[0][2] == pixman_int_to_fixed(3));
    assert(product.matrix[1][2] == pixman_int_to_fixed(5));

    /* Applying the translate to point (10, 20) yields (13, 25). */
    pixman_vector_t v = {
        { pixman_int_to_fixed(10),
          pixman_int_to_fixed(20),
          pixman_int_to_fixed(1) }
    };
    ok = pixman_transform_point(&translate, &v);
    assert(ok);
    assert(v.vector[0] == pixman_int_to_fixed(13));
    assert(v.vector[1] == pixman_int_to_fixed(25));
    assert(v.vector[2] == pixman_int_to_fixed(1));

    printf("pixman transform test OK\n");
    return 0;
}
