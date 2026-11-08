/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmpdec: mul cmp; calls mpd_defaultcontext(), mpd_new(), mpd_qset_string(), mpd_qmul().
 */

#include <mpdecimal.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    mpd_context_t ctx;
    mpd_defaultcontext(&ctx);

    mpd_t *a = mpd_new(&ctx);
    mpd_t *b = mpd_new(&ctx);
    mpd_t *prod = mpd_new(&ctx);
    mpd_t *expected = mpd_new(&ctx);
    assert(a && b && prod && expected);

    uint32_t status = 0;
    mpd_qset_string(a, "0.1", &ctx, &status);
    mpd_qset_string(b, "3", &ctx, &status);
    mpd_qmul(prod, a, b, &ctx, &status);
    assert(status == 0);

    char *s = mpd_to_sci(prod, 1);
    assert(s != NULL);
    assert(strcmp(s, "0.3") == 0);
    mpd_free(s);

    /* Comparison: prod (0.3) vs expected (0.3) must be 0; vs 0.5 must be -1; vs 0.1 must be +1. */
    mpd_qset_string(expected, "0.3", &ctx, &status);
    assert(status == 0);
    int cmp_eq = mpd_qcmp(prod, expected, &status);
    assert(status == 0);
    assert(cmp_eq == 0);

    mpd_t *bigger = mpd_new(&ctx);
    mpd_t *smaller = mpd_new(&ctx);
    assert(bigger && smaller);
    mpd_qset_string(bigger, "0.5", &ctx, &status);
    mpd_qset_string(smaller, "0.1", &ctx, &status);
    assert(status == 0);

    int cmp_lt = mpd_qcmp(prod, bigger, &status);
    int cmp_gt = mpd_qcmp(prod, smaller, &status);
    assert(status == 0);
    assert(cmp_lt < 0);
    assert(cmp_gt > 0);

    mpd_del(a);
    mpd_del(b);
    mpd_del(prod);
    mpd_del(expected);
    mpd_del(bigger);
    mpd_del(smaller);

    printf("libmpdec mul+cmp test OK\n");
    return 0;
}
