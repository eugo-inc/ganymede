/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libaio: core API; calls io_setup(), io_prep_pread(), io_submit(), io_getevents().
 */

#include <libaio.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void) {
    char path[] = "/tmp/demo_libaio_probe_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    unlink(path);

    static const char payload[] = "demo-libaio-probe";
    if (write(fd, payload, sizeof payload) != (ssize_t) sizeof payload) {
        perror("write");
        return 1;
    }

    io_context_t context = 0;
    int rc = io_setup(4, &context);
    if (rc != 0) {
        fprintf(stderr, "libaio: io_setup failed: %s\n", strerror(-rc));
        return 1;
    }

    char buffer[sizeof payload];
    memset(buffer, 0, sizeof buffer);
    struct iocb request;
    struct iocb *requests[1] = {&request};
    io_prep_pread(&request, fd, buffer, sizeof buffer, 0);

    rc = io_submit(context, 1, requests);
    if (rc != 1) {
        fprintf(stderr, "libaio: io_submit returned %d\n", rc);
        return 1;
    }

    struct io_event event;
    rc = io_getevents(context, 1, 1, &event, NULL);
    if (rc != 1) {
        fprintf(stderr, "libaio: io_getevents returned %d\n", rc);
        return 1;
    }
    if (event.obj != &request || event.res != sizeof payload || memcmp(buffer, payload, sizeof payload) != 0) {
        fprintf(stderr, "libaio: the completed read does not match (res=%lu)\n", (unsigned long) event.res);
        return 1;
    }

    io_destroy(context);
    close(fd);
    printf("libaio: an asynchronous pread of %zu bytes round-tripped\n", sizeof payload);
    return 0;
}
