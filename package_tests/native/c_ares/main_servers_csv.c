/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for c_ares: servers csv; calls ares_library_init(), ares_init_options(), ares_set_servers_csv(), ares_get_servers_csv().
 */

#include <ares.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS);

    ares_channel_t *channel = NULL;
    struct ares_options opts;
    memset(&opts, 0, sizeof(opts));
    assert(ares_init_options(&channel, &opts, 0) == ARES_SUCCESS);
    assert(channel != NULL);

    /* --- Set a known list of public DNS resolvers. No network is touched;
     *     this is purely channel-config bookkeeping. */
    const char *want = "8.8.8.8,1.1.1.1,9.9.9.9";
    int rc = ares_set_servers_csv(channel, want);
    assert(rc == ARES_SUCCESS);

    /* --- Read the list back; getter must produce a non-empty CSV. */
    char *got = ares_get_servers_csv(channel);
    assert(got != NULL);
    assert(strlen(got) > 0);

    assert(strstr(got, "8.8.8.8") != NULL);
    assert(strstr(got, "1.1.1.1") != NULL);
    assert(strstr(got, "9.9.9.9") != NULL);

    ares_free_string(got);

    rc = ares_set_servers_csv(channel, "this is not an ip address");
    assert(rc != ARES_SUCCESS);

    ares_destroy(channel);
    ares_library_cleanup();

    printf("c_ares servers_csv test OK\n");
    return 0;
}
