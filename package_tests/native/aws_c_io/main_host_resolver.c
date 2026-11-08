/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_io: host resolver; calls aws_default_allocator(), aws_io_library_init(), aws_event_loop_group_new_default(), aws_host_resolver_new_default().
 */

#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/host_resolver.h>
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
        .shutdown_options = NULL,
        .system_clock_override_fn = NULL,
    };
    struct aws_host_resolver *resolver = aws_host_resolver_new_default(alloc, &resolver_options);
    assert(resolver != NULL);

    aws_host_resolver_release(resolver);
    aws_event_loop_group_release(elg);

    aws_io_library_clean_up();

    printf("aws_c_io host_resolver test OK\n");
    return 0;
}
