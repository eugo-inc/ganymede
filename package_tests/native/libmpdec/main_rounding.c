/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmpdec: rounding; calls mpd_defaultcontext(), mpd_qsetround(), mpd_new(), mpd_qset_string().
 */

#include <mpdecimal.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void round_with_mode(int mode, const char *input, const char *expected) {
    mpd_context_t ctx;
    mpd_defaultcontext(&ctx);
    int ok = mpd_qsetround(&ctx, mode);
    assert(ok == 1);

    mpd_t *x = mpd_new(&ctx);
    mpd_t *r = mpd_new(&ctx);
    assert(x && r);

    uint32_t status = 0;
    mpd_qset_string(x, input, &ctx, &status);
    assert(status == 0);

    mpd_qround_to_int(r, x, &ctx, &status);

    char *s = mpd_to_sci(r, 1);
    assert(s != NULL);
    assert(strcmp(s, expected) == 0);
    mpd_free(s);

    mpd_del(x);
    mpd_del(r);
}

int main(void) {
    /* 2.5 — the case where HALF_EVEN ("banker's") and HALF_UP disagree. */
    round_with_mode(MPD_ROUND_HALF_EVEN, "2.5", "2");
    round_with_mode(MPD_ROUND_HALF_UP, "2.5", "3");

    /* -1.5 — FLOOR rounds toward -inf, CEILING toward +inf. */
    round_with_mode(MPD_ROUND_FLOOR, "-1.5", "-2");
    round_with_mode(MPD_ROUND_CEILING, "-1.5", "-1");

    /* 1.4 — HALF_EVEN drops the .4 to 1; CEILING bumps it to 2. */
    round_with_mode(MPD_ROUND_HALF_EVEN, "1.4", "1");
    round_with_mode(MPD_ROUND_CEILING, "1.4", "2");

    printf("libmpdec rounding test OK\n");
    return 0;
}
