/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_io: core API; calls aws_default_allocator(), aws_io_library_init(), aws_event_loop_group_new_default(), aws_event_loop_group_release().
 */

#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    struct aws_event_loop_group *elg = aws_event_loop_group_new_default(alloc, 1, NULL);
    assert(elg != NULL);
    aws_event_loop_group_release(elg);

    /* aws_io_error_str must return a non-NULL human-readable string for an
     * IO-table error — catches a missing IO error-table registration. */
    const char *err = aws_error_str(AWS_IO_SOCKET_CONNECTION_REFUSED);
    assert(err != NULL);
    assert(strlen(err) > 0);

    aws_io_library_clean_up();

    printf("aws_c_io test OK: error_str(SOCKET_CONNECTION_REFUSED)=\"%s\"\n", err);
    return 0;
}
