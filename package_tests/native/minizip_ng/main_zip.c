/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for minizip_ng: zip; calls mz_stream_mem_create(), mz_stream_mem_set_grow_size(), mz_stream_mem_open(), mz_zip_writer_create().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mz.h>
#include <mz_strm.h>
#include <mz_strm_mem.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>
#include <mz_os.h>     /* MZ_VERSION_MADEBY lives here, not in mz.h */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    const char *entry_name = "hello.txt";
    const char  payload[]  = "Hello from minizip-ng!";

    /* --- write a single-entry zip into an in-memory buffer ------------ */
    void *mem_stream = mz_stream_mem_create();
    assert(mem_stream != NULL);
    mz_stream_mem_set_grow_size(mem_stream, 4096);
    int32_t rc = mz_stream_mem_open(mem_stream, NULL, MZ_OPEN_MODE_CREATE);
    assert(rc == MZ_OK);

    void *writer = mz_zip_writer_create();
    assert(writer != NULL);

    rc = mz_zip_writer_open(writer, mem_stream, 0);
    assert(rc == MZ_OK);

    mz_zip_file file_info;
    memset(&file_info, 0, sizeof(file_info));
    file_info.version_madeby   = MZ_VERSION_MADEBY;
    file_info.flag             = MZ_ZIP_FLAG_UTF8;
    file_info.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
    file_info.filename         = entry_name;
    file_info.filename_size    = (uint16_t)strlen(entry_name);
    file_info.uncompressed_size = (int64_t)(sizeof(payload) - 1);

    rc = mz_zip_writer_add_buffer(writer, (void *)payload, (int32_t)(sizeof(payload) - 1), &file_info);
    assert(rc == MZ_OK);

    rc = mz_zip_writer_close(writer);
    assert(rc == MZ_OK);
    mz_zip_writer_delete(&writer);
    assert(writer == NULL);

    /* --- read it back -------------------------------------------------- */
    rc = mz_stream_mem_seek(mem_stream, 0, MZ_SEEK_SET);
    assert(rc == 0);

    void *reader = mz_zip_reader_create();
    assert(reader != NULL);

    rc = mz_zip_reader_open(reader, mem_stream);
    assert(rc == MZ_OK);

    rc = mz_zip_reader_goto_first_entry(reader);
    assert(rc == MZ_OK);

    mz_zip_file *info_out = NULL;
    rc = mz_zip_reader_entry_get_info(reader, &info_out);
    assert(rc == MZ_OK);
    assert(info_out != NULL);
    assert(info_out->filename != NULL);
    assert(strcmp(info_out->filename, entry_name) == 0);
    assert(info_out->uncompressed_size == (int64_t)(sizeof(payload) - 1));

    /* mz_zip_reader_entry_save_buffer requires the length to equal the entry's
     * uncompressed size exactly — a larger buffer returns MZ_BUF_ERROR. */
    char out_buf[sizeof(payload)] = {0};
    int32_t bytes = mz_zip_reader_entry_save_buffer(reader, out_buf, (int32_t)(sizeof(payload) - 1));
    assert(bytes == MZ_OK);
    assert(memcmp(out_buf, payload, sizeof(payload) - 1) == 0);

    rc = mz_zip_reader_close(reader);
    assert(rc == MZ_OK);
    mz_zip_reader_delete(&reader);
    assert(reader == NULL);

    mz_stream_mem_close(mem_stream);
    mz_stream_mem_delete(&mem_stream);

    printf("minizip_ng zip writer/reader round-trip OK\n");
    return 0;
}
