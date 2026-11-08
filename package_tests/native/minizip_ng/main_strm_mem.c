/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for minizip_ng: strm mem; calls mz_stream_mem_create(), mz_stream_mem_open(), mz_stream_mem_write(), mz_stream_mem_seek().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    void *stream = mz_stream_mem_create();
    assert(stream != NULL);

    int32_t rc = mz_stream_mem_open(stream, NULL, MZ_OPEN_MODE_CREATE);
    assert(rc == MZ_OK);

    const char payload[] = "minizip-ng memory stream round-trip";
    int32_t written = mz_stream_mem_write(stream, payload, (int32_t)sizeof(payload));
    assert(written == (int32_t)sizeof(payload));

    /* Seek back to the start. */
    int64_t pos = mz_stream_mem_seek(stream, 0, MZ_SEEK_SET);
    assert(pos == 0);

    char buffer[sizeof(payload)] = {0};
    int32_t read_n = mz_stream_mem_read(stream, buffer, (int32_t)sizeof(buffer));
    assert(read_n == (int32_t)sizeof(payload));
    assert(memcmp(buffer, payload, sizeof(payload)) == 0);

    /* Tell should report the post-read position. */
    int64_t tell = mz_stream_mem_tell(stream);
    assert(tell == (int64_t)sizeof(payload));

    mz_stream_mem_close(stream);
    mz_stream_mem_delete(&stream);
    assert(stream == NULL);

    printf("minizip_ng memory stream OK\n");
    return 0;
}
