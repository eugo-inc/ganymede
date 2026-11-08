/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzip: compression; calls zip_compression_method_supported(), mkstemps(), zip_open(), zip_source_buffer().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <zip.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
    /* DEFLATE must be reported as a supported compression method (mandatory). */
    assert(zip_compression_method_supported(ZIP_CM_DEFLATE, 1) == 1);
    /* STORE is always supported. */
    assert(zip_compression_method_supported(ZIP_CM_STORE, 1) == 1);

    char path[] = "/tmp/demo_libzip_compress_XXXXXX.zip";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);
    unlink(path);

    int err = 0;
    zip_t *za = zip_open(path, ZIP_CREATE | ZIP_EXCL, &err);
    assert(za != NULL);

    /* 4 KiB of a repeating 16-byte block — trivially compressible. */
    const zip_uint64_t payload_len = 4096;
    char *payload = (char *)malloc((size_t)payload_len);
    assert(payload != NULL);
    for (zip_uint64_t i = 0; i < payload_len; ++i) {
        payload[i] = (char)('A' + (i % 16));
    }

    /* Entry 1: stored. */
    zip_source_t *src_stored = zip_source_buffer(za, payload, payload_len, 0);
    assert(src_stored != NULL);
    zip_int64_t idx_stored = zip_file_add(za, "stored.bin", src_stored, ZIP_FL_OVERWRITE);
    assert(idx_stored >= 0);
    assert(zip_set_file_compression(za, (zip_uint64_t)idx_stored, ZIP_CM_STORE, 0) == 0);

    /* Entry 2: deflated. */
    zip_source_t *src_deflated = zip_source_buffer(za, payload, payload_len, 0);
    assert(src_deflated != NULL);
    zip_int64_t idx_deflated = zip_file_add(za, "deflated.bin", src_deflated, ZIP_FL_OVERWRITE);
    assert(idx_deflated >= 0);
    assert(zip_set_file_compression(za, (zip_uint64_t)idx_deflated, ZIP_CM_DEFLATE, 0) == 0);

    assert(zip_close(za) == 0);

    /* Reopen read-only and inspect the compressed/uncompressed sizes. */
    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);
    assert(zip_get_num_entries(za, 0) == 2);

    zip_int64_t i_s = zip_name_locate(za, "stored.bin", 0);
    zip_int64_t i_d = zip_name_locate(za, "deflated.bin", 0);
    assert(i_s >= 0 && i_d >= 0);

    zip_stat_t st_s, st_d;
    zip_stat_init(&st_s);
    zip_stat_init(&st_d);
    assert(zip_stat_index(za, (zip_uint64_t)i_s, 0, &st_s) == 0);
    assert(zip_stat_index(za, (zip_uint64_t)i_d, 0, &st_d) == 0);

    /* Both must report a valid compressed size + method. */
    assert((st_s.valid & ZIP_STAT_COMP_SIZE) != 0);
    assert((st_s.valid & ZIP_STAT_COMP_METHOD) != 0);
    assert((st_d.valid & ZIP_STAT_COMP_SIZE) != 0);
    assert((st_d.valid & ZIP_STAT_COMP_METHOD) != 0);

    assert(st_s.comp_method == ZIP_CM_STORE);
    assert(st_d.comp_method == ZIP_CM_DEFLATE);

    /* Uncompressed size matches the payload for both. */
    assert(st_s.size == payload_len);
    assert(st_d.size == payload_len);

    /* STORE: compressed size equals uncompressed size. */
    assert(st_s.comp_size == payload_len);

    /* DEFLATE: compressed size strictly less than uncompressed (verifies the
     * deflate backend ran AND produced compression on this trivially-redundant
     * input). If the deflate backend were broken, this would either fail at
     * zip_close or produce comp_size >= size. */
    assert(st_d.comp_size < payload_len);

    zip_file_t *zf = zip_fopen_index(za, (zip_uint64_t)i_d, 0);
    assert(zf != NULL);
    char *out = (char *)malloc((size_t)payload_len);
    assert(out != NULL);
    zip_int64_t got = zip_fread(zf, out, payload_len);
    assert(got == (zip_int64_t)payload_len);
    assert(memcmp(out, payload, payload_len) == 0);
    zip_fclose(zf);
    free(out);

    zip_close(za);
    unlink(path);
    free(payload);

    printf("libzip compression test OK (deflated %llu -> %llu bytes)\n",
           (unsigned long long)payload_len,
           (unsigned long long)st_d.comp_size);
    return 0;
}
