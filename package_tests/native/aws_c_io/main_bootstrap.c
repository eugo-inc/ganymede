/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_io: bootstrap; calls aws_default_allocator(), aws_io_library_init(), aws_event_loop_group_new_default(), aws_host_resolver_new_default().
 */

#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/host_resolver.h>
#include <aws/io/channel_bootstrap.h>
#include <aws/common/allocator.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    struct aws_event_loop_group *elg = aws_event_loop_group_new_default(alloc, 1, NULL);
    assert(elg != NULL);

    struct aws_host_resolver_default_options resolver_options = {
        .max_entries = 8,
        .el_group = elg,
    };
    struct aws_host_resolver *resolver = aws_host_resolver_new_default(alloc, &resolver_options);
    assert(resolver != NULL);

    struct aws_client_bootstrap_options bootstrap_options = {
        .event_loop_group = elg,
        .host_resolver = resolver,
        .host_resolution_config = NULL,
        .on_shutdown_complete = NULL,
        .user_data = NULL,
    };
    struct aws_client_bootstrap *bootstrap = aws_client_bootstrap_new(alloc, &bootstrap_options);
    assert(bootstrap != NULL);

    aws_client_bootstrap_release(bootstrap);
    aws_host_resolver_release(resolver);
    aws_event_loop_group_release(elg);

    aws_io_library_clean_up();

    printf("aws_c_io client_bootstrap test OK\n");
    return 0;
}
