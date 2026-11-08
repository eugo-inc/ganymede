/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for c_ares: dup; calls ares_library_init(), ares_init_options(), ares_set_servers_csv(), ares_dup().
 */

#include <ares.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS);

    ares_channel_t *src = NULL;
    struct ares_options opts;
    memset(&opts, 0, sizeof(opts));
    opts.timeout = 1500;
    opts.tries   = 2;
    const int optmask = ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES;
    assert(ares_init_options(&src, &opts, optmask) == ARES_SUCCESS);
    assert(src != NULL);

    assert(ares_set_servers_csv(src, "8.8.8.8,1.1.1.1") == ARES_SUCCESS);

    /* --- Duplicate the channel. */
    ares_channel_t *dup = NULL;
    int rc = ares_dup(&dup, src);
    assert(rc == ARES_SUCCESS);
    assert(dup != NULL);
    assert(dup != src);   /* must be a different object */

    /* --- The dup must inherit the server list. */
    char *dup_servers = ares_get_servers_csv(dup);
    assert(dup_servers != NULL);
    assert(strlen(dup_servers) > 0);
    assert(strstr(dup_servers, "8.8.8.8") != NULL);
    assert(strstr(dup_servers, "1.1.1.1") != NULL);
    ares_free_string(dup_servers);

    /* --- Destroy the source FIRST. The dup must still be usable —
     *     catches a shallow dup that shares heap pointers with src. */
    ares_destroy(src);

    /* Re-query the dup's servers after src is gone; must still work. */
    char *post = ares_get_servers_csv(dup);
    assert(post != NULL);
    assert(strstr(post, "8.8.8.8") != NULL);
    ares_free_string(post);

    ares_destroy(dup);
    ares_library_cleanup();

    printf("c_ares dup test OK\n");
    return 0;
}
