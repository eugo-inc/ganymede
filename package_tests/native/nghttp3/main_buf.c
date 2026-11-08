/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp3: buf; calls nghttp3_buf_init(), nghttp3_buf_len(), nghttp3_buf_left(), nghttp3_buf_reset().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    nghttp3_buf buf;

    /* 1) Fresh init: every offset must be NULL/zero. */
    nghttp3_buf_init(&buf);
    assert(buf.begin == NULL);
    assert(buf.end == NULL);
    assert(buf.pos == NULL);
    assert(buf.last == NULL);
    assert(nghttp3_buf_len(&buf) == 0);
    assert(nghttp3_buf_left(&buf) == 0);

    /* 2) Attach caller-owned storage: empty, with the full capacity left. */
    unsigned char storage[64];
    buf.begin = storage;
    buf.end = storage + sizeof storage;
    buf.pos = storage;
    buf.last = storage;

    assert(nghttp3_buf_len(&buf) == 0);
    assert(nghttp3_buf_left(&buf) == sizeof storage);

    /* 3) Simulate appending 10 bytes: len grows, left shrinks by the same. */
    buf.last += 10;
    assert(nghttp3_buf_len(&buf) == 10);
    assert(nghttp3_buf_left(&buf) == sizeof storage - 10);

    /* 4) Simulate consuming 4 bytes: len shrinks, left is unchanged
     *    (left is end-last, not end-pos). */
    buf.pos += 4;
    assert(nghttp3_buf_len(&buf) == 6);
    assert(nghttp3_buf_left(&buf) == sizeof storage - 10);

    /* 5) Reset: pos and last both rewind to begin; len and left return
     *    to the empty-but-allocated state. */
    nghttp3_buf_reset(&buf);
    assert(buf.pos == storage);
    assert(buf.last == storage);
    assert(nghttp3_buf_len(&buf) == 0);
    assert(nghttp3_buf_left(&buf) == sizeof storage);

    printf("nghttp3 buf test OK\n");
    return 0;
}
