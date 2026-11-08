/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for minizip_ng: strm buf; calls mz_stream_mem_create(), mz_stream_mem_set_grow_size(), mz_stream_mem_open(), mz_stream_buffered_create().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_buf.h>
#include <mz_strm_mem.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    /* Base: in-memory stream. */
    void *base = mz_stream_mem_create();
    assert(base != NULL);
    mz_stream_mem_set_grow_size(base, 4096);
    int32_t rc = mz_stream_mem_open(base, NULL, MZ_OPEN_MODE_CREATE);
    assert(rc == MZ_OK);

    /* --- write path: buffered_stream wraps base for writes. */
    void *buf_w = mz_stream_buffered_create();
    assert(buf_w != NULL);
    mz_stream_set_base(buf_w, base);

    rc = mz_stream_buffered_open(buf_w, NULL, MZ_OPEN_MODE_WRITE);
    assert(rc == MZ_OK);

    const char payload[] = "The buffered stream batches small writes "
                           "into the base stream; this string is long "
                           "enough to span the buffer at least once.";
    int32_t written = mz_stream_buffered_write(buf_w, payload, (int32_t)sizeof(payload));
    assert(written == (int32_t)sizeof(payload));

    int64_t tell_w = mz_stream_buffered_tell(buf_w);
    assert(tell_w == (int64_t)sizeof(payload));

    rc = mz_stream_buffered_close(buf_w);
    assert(rc == MZ_OK);
    mz_stream_buffered_delete(&buf_w);
    assert(buf_w == NULL);

    /* --- read path: rewind base, layer a fresh buffered stream for reads. */
    rc = mz_stream_mem_seek(base, 0, MZ_SEEK_SET);
    assert(rc == 0);

    void *buf_r = mz_stream_buffered_create();
    assert(buf_r != NULL);
    mz_stream_set_base(buf_r, base);

    rc = mz_stream_buffered_open(buf_r, NULL, MZ_OPEN_MODE_READ);
    assert(rc == MZ_OK);

    /* Read in two small chunks to exercise the buffer refill path. */
    char chunk1[16] = {0};
    int32_t read1 = mz_stream_buffered_read(buf_r, chunk1, (int32_t)sizeof(chunk1));
    assert(read1 == (int32_t)sizeof(chunk1));
    assert(memcmp(chunk1, payload, sizeof(chunk1)) == 0);

    char rest[sizeof(payload) - 16] = {0};
    int32_t read2 = mz_stream_buffered_read(buf_r, rest, (int32_t)sizeof(rest));
    assert(read2 == (int32_t)sizeof(rest));
    assert(memcmp(rest, payload + 16, sizeof(rest)) == 0);

    int64_t tell_r = mz_stream_buffered_tell(buf_r);
    assert(tell_r == (int64_t)sizeof(payload));

    rc = mz_stream_buffered_close(buf_r);
    assert(rc == MZ_OK);
    mz_stream_buffered_delete(&buf_r);

    mz_stream_mem_close(base);
    mz_stream_mem_delete(&base);

    printf("minizip_ng buffered stream OK\n");
    return 0;
}
