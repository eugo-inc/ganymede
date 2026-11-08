/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for minizip_ng: strm zlib; calls mz_stream_mem_create(), mz_stream_mem_set_grow_size(), mz_stream_mem_open(), mz_stream_zlib_create().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_strm_zlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    /* A buffer that is highly compressible — repeating ASCII. */
    char input[2048];
    for (size_t i = 0; i < sizeof(input); i++)
        input[i] = (char)('A' + (i % 26));

    /* --- compress: input bytes -> deflate stream -> compressed_mem. */
    void *compressed_mem = mz_stream_mem_create();
    assert(compressed_mem != NULL);
    mz_stream_mem_set_grow_size(compressed_mem, 4096);
    int32_t rc = mz_stream_mem_open(compressed_mem, NULL, MZ_OPEN_MODE_CREATE);
    assert(rc == MZ_OK);

    void *deflate_stream = mz_stream_zlib_create();
    assert(deflate_stream != NULL);
    mz_stream_set_base(deflate_stream, compressed_mem);

    rc = mz_stream_open(deflate_stream, NULL, MZ_OPEN_MODE_WRITE);
    assert(rc == MZ_OK);
    int32_t written = mz_stream_write(deflate_stream, input, (int32_t)sizeof(input));
    assert(written == (int32_t)sizeof(input));
    rc = mz_stream_close(deflate_stream);
    assert(rc == MZ_OK);

    int64_t total_in = 0;
    int64_t total_out = 0;
    mz_stream_get_prop_int64(deflate_stream, MZ_STREAM_PROP_TOTAL_IN, &total_in);
    mz_stream_get_prop_int64(deflate_stream, MZ_STREAM_PROP_TOTAL_OUT, &total_out);
    assert(total_in == (int64_t)sizeof(input));
    /* Compressed size must be strictly smaller than the highly-repetitive
     * input — otherwise the deflate backend is broken or silently storing. */
    assert(total_out > 0);
    assert(total_out < (int64_t)sizeof(input));
    mz_stream_zlib_delete(&deflate_stream);
    assert(deflate_stream == NULL);

    /* --- decompress: compressed_mem -> inflate stream -> roundtrip_buf. */
    rc = mz_stream_mem_seek(compressed_mem, 0, MZ_SEEK_SET);
    assert(rc == 0);

    void *inflate_stream = mz_stream_zlib_create();
    assert(inflate_stream != NULL);
    mz_stream_set_base(inflate_stream, compressed_mem);

    rc = mz_stream_open(inflate_stream, NULL, MZ_OPEN_MODE_READ);
    assert(rc == MZ_OK);

    char roundtrip[sizeof(input)] = {0};
    int32_t read_n = mz_stream_read(inflate_stream, roundtrip, (int32_t)sizeof(roundtrip));
    assert(read_n == (int32_t)sizeof(input));
    assert(memcmp(roundtrip, input, sizeof(input)) == 0);

    rc = mz_stream_close(inflate_stream);
    assert(rc == MZ_OK);
    mz_stream_zlib_delete(&inflate_stream);

    mz_stream_mem_close(compressed_mem);
    mz_stream_mem_delete(&compressed_mem);

    printf("minizip_ng zlib deflate/inflate round-trip OK\n");
    return 0;
}
