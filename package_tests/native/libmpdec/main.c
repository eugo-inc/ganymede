/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmpdec: core API; calls mpd_defaultcontext(), mpd_new(), mpd_qset_string(), mpd_qadd().
 */

#include <mpdecimal.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    mpd_context_t ctx;
    mpd_defaultcontext(&ctx);

    mpd_t *a = mpd_new(&ctx);
    mpd_t *b = mpd_new(&ctx);
    mpd_t *sum = mpd_new(&ctx);
    assert(a != NULL && b != NULL && sum != NULL);

    uint32_t status = 0;
    mpd_qset_string(a, "0.1", &ctx, &status);
    mpd_qset_string(b, "0.2", &ctx, &status);
    mpd_qadd(sum, a, b, &ctx, &status);
    assert(status == 0);

    char *s = mpd_to_sci(sum, 1);
    assert(s != NULL);
    assert(strcmp(s, "0.3") == 0);

    mpd_free(s);
    mpd_del(a);
    mpd_del(b);
    mpd_del(sum);

    printf("libmpdec test OK: %s\n", mpd_version());
    return 0;
}
