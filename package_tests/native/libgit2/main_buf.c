/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libgit2: buf; calls git_libgit2_init(), git_buf_set(), git_buf_dispose(), git_libgit2_shutdown().
 */

#include <git2.h>
#include <git2/buffer.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(git_libgit2_init() >= 1);

    /* Zero-initialize the buffer (libgit2 convention). */
    git_buf buf = GIT_BUF_INIT;
    assert(buf.ptr == NULL);
    assert(buf.size == 0);

    /* Set buffer contents from a C string with explicit length. */
    const char *payload = "hello libgit2 buffer";
    size_t payload_len = strlen(payload);
    assert(git_buf_set(&buf, payload, payload_len) == 0);

    assert(buf.ptr != NULL);
    assert(buf.size == payload_len);
    assert(memcmp(buf.ptr, payload, payload_len) == 0);
    assert(buf.ptr[buf.size] == '\0');

    /* Overwrite with shorter content; size must shrink. */
    assert(git_buf_set(&buf, "short", 5) == 0);
    assert(buf.size == 5);
    assert(memcmp(buf.ptr, "short", 5) == 0);

    /* Release the buffer's memory. dispose is idempotent in v1.x. */
    git_buf_dispose(&buf);
    git_buf_dispose(&buf);

    assert(git_libgit2_shutdown() == 0);
    printf("libgit2 buf test OK\n");
    return 0;
}
