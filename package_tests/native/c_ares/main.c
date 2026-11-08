/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for c_ares: core API; calls ares_library_init(), ares_init(), ares_destroy(), ares_library_cleanup().
 */

#include <ares.h>
#include <ares_version.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS);

    ares_channel channel = NULL;
    assert(ares_init(&channel) == ARES_SUCCESS);
    assert(channel != NULL);

    ares_destroy(channel);
    ares_library_cleanup();

    /* Version string must be non-empty and match the major version macro. */
    int version_num = 0;
    const char *v = ares_version(&version_num);
    assert(v != NULL);
    assert(strlen(v) > 0);
    assert(version_num == ARES_VERSION);

    printf("c_ares test OK: %s\n", v);
    return 0;
}
