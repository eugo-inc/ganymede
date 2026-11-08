/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for c_ares: init options; calls ares_library_init(), ares_init_options(), ares_save_options(), ares_destroy_options().
 */

#include <ares.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS);

    struct ares_options opts;
    memset(&opts, 0, sizeof(opts));
    opts.timeout = 2500;
    opts.tries   = 3;
    const int optmask = ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES;

    ares_channel_t *channel = NULL;
    int rc = ares_init_options(&channel, &opts, optmask);
    assert(rc == ARES_SUCCESS);
    assert(channel != NULL);

    struct ares_options saved;
    memset(&saved, 0, sizeof(saved));
    int saved_mask = 0;
    rc = ares_save_options(channel, &saved, &saved_mask);
    assert(rc == ARES_SUCCESS);

    assert((saved_mask & ARES_OPT_TIMEOUTMS) != 0);
    assert((saved_mask & ARES_OPT_TRIES)     != 0);

    assert(saved.timeout == 2500);
    assert(saved.tries   == 3);

    ares_destroy_options(&saved);
    ares_destroy(channel);
    ares_library_cleanup();

    printf("c_ares init_options test OK: timeout=%d tries=%d mask=0x%x\n",
           saved.timeout, saved.tries, saved_mask);
    return 0;
}
