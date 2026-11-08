/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmpdec: div inexact; calls mpd_defaultcontext(), mpd_qsetprec(), mpd_new(), mpd_qset_string().
 */

#include <mpdecimal.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    mpd_context_t ctx;
    mpd_defaultcontext(&ctx);

    int ok = mpd_qsetprec(&ctx, 9);
    assert(ok == 1);

    mpd_t *one = mpd_new(&ctx);
    mpd_t *three = mpd_new(&ctx);
    mpd_t *q = mpd_new(&ctx);
    assert(one && three && q);

    uint32_t status = 0;
    mpd_qset_string(one, "1", &ctx, &status);
    mpd_qset_string(three, "3", &ctx, &status);
    assert(status == 0);

    mpd_qdiv(q, one, three, &ctx, &status);

    /* Both Inexact and Rounded MUST fire for 1/3 at finite precision. */
    assert((status & MPD_Inexact) != 0);
    assert((status & MPD_Rounded) != 0);
    /* No catastrophic error flags should be present. */
    assert((status & MPD_Division_by_zero) == 0);
    assert((status & MPD_Division_undefined) == 0);
    assert((status & MPD_Division_impossible) == 0);

    char *s = mpd_to_sci(q, 1);
    assert(s != NULL);
    assert(strcmp(s, "0.333333333") == 0);
    mpd_free(s);

    /* Verify divide-by-zero sets the matching flag (and only on a fresh status mask). */
    mpd_t *zero = mpd_new(&ctx);
    mpd_t *r = mpd_new(&ctx);
    assert(zero && r);
    uint32_t dz_status = 0;
    mpd_qset_string(zero, "0", &ctx, &dz_status);
    assert(dz_status == 0);
    mpd_qdiv(r, one, zero, &ctx, &dz_status);
    assert((dz_status & MPD_Division_by_zero) != 0);

    mpd_del(one);
    mpd_del(three);
    mpd_del(q);
    mpd_del(zero);
    mpd_del(r);

    printf("libmpdec div+inexact test OK\n");
    return 0;
}
