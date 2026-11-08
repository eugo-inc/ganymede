/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xnnpack: weights cache; calls xnn_initialize(), xnn_create_weights_cache(), xnn_weights_cache_is_finalized(), xnn_finalize_weights_cache().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

int main(void) {
    enum xnn_status init_status = xnn_initialize(NULL);
    assert(init_status == xnn_status_success);

    xnn_weights_cache_t cache = NULL;
    enum xnn_status create_status = xnn_create_weights_cache(&cache);
    assert(create_status == xnn_status_success);
    assert(cache != NULL);

    /* Freshly-created cache must NOT be finalized. */
    bool finalized_before = xnn_weights_cache_is_finalized(cache);
    assert(!finalized_before);

    /* Soft finalization: the cache can still be queried but no further
     * packed-weight blobs can be appended. */
    enum xnn_status finalize_status = xnn_finalize_weights_cache(
        cache,
        xnn_weights_cache_finalization_kind_soft);
    assert(finalize_status == xnn_status_success);

    bool finalized_after = xnn_weights_cache_is_finalized(cache);
    assert(finalized_after);

    enum xnn_status delete_status = xnn_delete_weights_cache(cache);
    assert(delete_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    printf("xnnpack weights_cache test OK\n");
    return 0;
}
