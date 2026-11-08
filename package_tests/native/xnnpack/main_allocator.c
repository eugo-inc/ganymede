/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xnnpack: allocator; calls posix_memalign(), xnn_initialize(), xnn_create_subgraph(), xnn_create_workspace().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct probe_ctx {
    uint64_t allocate_calls;
    uint64_t reallocate_calls;
    uint64_t deallocate_calls;
    uint64_t aligned_allocate_calls;
    uint64_t aligned_deallocate_calls;
};

static void* probe_allocate(void* context, size_t size) {
    ((struct probe_ctx*)context)->allocate_calls++;
    return malloc(size);
}

static void* probe_reallocate(void* context, void* pointer, size_t size) {
    ((struct probe_ctx*)context)->reallocate_calls++;
    return realloc(pointer, size);
}

static void probe_deallocate(void* context, void* pointer) {
    ((struct probe_ctx*)context)->deallocate_calls++;
    free(pointer);
}

static void* probe_aligned_allocate(void* context, size_t alignment, size_t size) {
    ((struct probe_ctx*)context)->aligned_allocate_calls++;
    size_t rounded = (size + alignment - 1) & ~(alignment - 1);
    void* p = NULL;
    if (posix_memalign(&p, alignment, rounded) != 0) {
        return NULL;
    }
    return p;
}

static void probe_aligned_deallocate(void* context, void* pointer) {
    ((struct probe_ctx*)context)->aligned_deallocate_calls++;
    free(pointer);
}

int main(void) {
    struct probe_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));

    struct xnn_allocator allocator = {
        .context = &ctx,
        .allocate = probe_allocate,
        .reallocate = probe_reallocate,
        .deallocate = probe_deallocate,
        .aligned_allocate = probe_aligned_allocate,
        .aligned_deallocate = probe_aligned_deallocate,
    };

    enum xnn_status init_status = xnn_initialize(&allocator);
    assert(init_status == xnn_status_success);

    xnn_subgraph_t subgraph = NULL;
    enum xnn_status sg_status = xnn_create_subgraph(
        /*external_value_ids=*/1,
        /*flags=*/0,
        &subgraph);
    assert(sg_status == xnn_status_success);
    assert(subgraph != NULL);

    xnn_workspace_t workspace = NULL;
    enum xnn_status ws_status = xnn_create_workspace(&workspace);
    assert(ws_status == xnn_status_success);
    assert(workspace != NULL);

    enum xnn_status ws_release_status = xnn_release_workspace(workspace);
    assert(ws_release_status == xnn_status_success);

    enum xnn_status sg_delete_status = xnn_delete_subgraph(subgraph);
    assert(sg_delete_status == xnn_status_success);

    enum xnn_status deinit_status = xnn_deinitialize();
    assert(deinit_status == xnn_status_success);

    uint64_t total_calls =
        ctx.allocate_calls + ctx.reallocate_calls +
        ctx.aligned_allocate_calls;
    assert(total_calls > 0);

    printf("xnnpack allocator test OK "
           "(alloc=%llu realloc=%llu free=%llu aligned_alloc=%llu aligned_free=%llu)\n",
           (unsigned long long)ctx.allocate_calls,
           (unsigned long long)ctx.reallocate_calls,
           (unsigned long long)ctx.deallocate_calls,
           (unsigned long long)ctx.aligned_allocate_calls,
           (unsigned long long)ctx.aligned_deallocate_calls);
    return 0;
}
