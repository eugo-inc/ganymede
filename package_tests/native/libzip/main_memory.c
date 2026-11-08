/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzip: memory; calls zip_error_init(), zip_source_buffer_create(), zip_source_keep(), zip_open_from_source().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zip.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    zip_error_t ze;
    zip_error_init(&ze);

    zip_source_t *src = zip_source_buffer_create(NULL, 0, 0, &ze);
    assert(src != NULL);

    zip_source_keep(src);

    zip_t *za = zip_open_from_source(src, ZIP_TRUNCATE, &ze);
    assert(za != NULL);

    static const char payload[] = "in-memory libzip payload";
    const zip_uint64_t payload_len = sizeof(payload) - 1;

    zip_source_t *entry_src = zip_source_buffer(za, payload, payload_len, 0);
    assert(entry_src != NULL);
    assert(zip_file_add(za, "mem.txt", entry_src, ZIP_FL_OVERWRITE) >= 0);

    /* Flush the archive into the backing source. */
    assert(zip_close(za) == 0);

    /* Read the raw archive bytes back from the source. */
    assert(zip_source_open(src) == 0);

    /* Stat the source to learn its size. */
    zip_stat_t st;
    zip_stat_init(&st);
    assert(zip_source_stat(src, &st) == 0);
    assert((st.valid & ZIP_STAT_SIZE) != 0);
    assert(st.size > 0);

    char *raw = (char *)malloc((size_t)st.size);
    assert(raw != NULL);
    zip_int64_t got = zip_source_read(src, raw, st.size);
    assert(got == (zip_int64_t)st.size);
    zip_source_close(src);
    zip_source_free(src);

    assert(st.size >= 4);
    assert(raw[0] == 'P' && raw[1] == 'K' && raw[2] == 0x03 && raw[3] == 0x04);

    zip_error_t ze2;
    zip_error_init(&ze2);
    zip_source_t *src2 = zip_source_buffer_create(raw, (zip_uint64_t)st.size, 0, &ze2);
    assert(src2 != NULL);

    zip_t *za2 = zip_open_from_source(src2, ZIP_RDONLY, &ze2);
    assert(za2 != NULL);
    assert(zip_get_num_entries(za2, 0) == 1);

    zip_file_t *zf = zip_fopen(za2, "mem.txt", 0);
    assert(zf != NULL);
    char buf[64] = {0};
    zip_int64_t n = zip_fread(zf, buf, sizeof(buf) - 1);
    assert(n == (zip_int64_t)payload_len);
    assert(memcmp(buf, payload, payload_len) == 0);
    zip_fclose(zf);

    assert(zip_close(za2) == 0);
    free(raw);

    zip_error_fini(&ze);
    zip_error_fini(&ze2);

    printf("libzip in-memory archive test OK\n");
    return 0;
}
