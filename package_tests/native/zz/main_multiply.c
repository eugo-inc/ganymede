/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zz: multiply; calls zz_setup(), zz_init(), zz_set_i64(), zz_mul().
 */

#include <stdio.h>
#include <string.h>
#include <zz/zz.h>

int main(void) {
    zz_t a, b, c;
    char text[128];

    if (zz_setup() != ZZ_OK) {
        return 1;
    }
    if (zz_init(&a) != ZZ_OK || zz_init(&b) != ZZ_OK || zz_init(&c) != ZZ_OK) {
        return 2;
    }
    if (zz_set_i64(9223372036854775807LL, &a) != ZZ_OK || zz_set_i64(9223372036854775807LL, &b) != ZZ_OK) {
        return 3;
    }
    if (zz_mul(&a, &b, &c) != ZZ_OK || zz_get_str(&c, 10, text) != ZZ_OK) {
        return 4;
    }
    /* (2^63 - 1)^2 */
    if (strcmp(text, "85070591730234615847396907784232501249") != 0) {
        fprintf(stderr, "unexpected product %s\n", text);
        return 5;
    }
    printf("zz product ok: %s\n", text);
    zz_clear(&a);
    zz_clear(&b);
    zz_clear(&c);
    zz_finish();
    return 0;
}
