/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libevent: core API; calls event_base_new(), event_base_free(), event_get_version().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <event2/event.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct event_base *base = event_base_new();
    assert(base != NULL);
    event_base_free(base);
    printf("libevent test OK: %s\n", event_get_version());
    return 0;
}
