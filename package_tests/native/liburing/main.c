/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for liburing: core API; calls io_uring_major_version(), io_uring_minor_version().
 */

#include <liburing.h>

#include <stdio.h>

int main(void) {
    const int major = io_uring_major_version();
    const int minor = io_uring_minor_version();

    if (major != IO_URING_VERSION_MAJOR || minor != IO_URING_VERSION_MINOR) {
        fprintf(stderr, "liburing: library %d.%d but header %d.%d\n", major, minor, IO_URING_VERSION_MAJOR,
                IO_URING_VERSION_MINOR);
        return 1;
    }

    printf("liburing: header and library agree on %d.%d\n", major, minor);
    return 0;
}
