/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libzip: core API; calls zip_libzip_version(), mkstemps(), zip_open(), zip_source_buffer().
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
    /* libzip version string must be present (documented API contract). */
    const char *v = zip_libzip_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    char path[] = "/tmp/demo_libzip_test_XXXXXX.zip";
    int fd = mkstemps(path, 4);
    assert(fd >= 0);
    close(fd);
    unlink(path);  /* zip_open with ZIP_CREATE | ZIP_EXCL wants the path absent. */

    int err = 0;
    zip_t *za = zip_open(path, ZIP_CREATE | ZIP_EXCL, &err);
    assert(za != NULL);
    assert(err == 0);

    static const char payload[] = "hello libzip";
    const zip_uint64_t payload_len = sizeof(payload) - 1;

    zip_source_t *src = zip_source_buffer(za, payload, payload_len, 0);
    assert(src != NULL);
    zip_int64_t idx = zip_file_add(za, "greeting.txt", src, ZIP_FL_OVERWRITE);
    assert(idx >= 0);

    /* zip_close flushes + writes the archive. */
    assert(zip_close(za) == 0);

    za = zip_open(path, ZIP_RDONLY, &err);
    assert(za != NULL);

    zip_int64_t num = zip_get_num_entries(za, 0);
    assert(num == 1);

    zip_file_t *zf = zip_fopen(za, "greeting.txt", 0);
    assert(zf != NULL);

    char buf[64] = {0};
    zip_int64_t n = zip_fread(zf, buf, sizeof(buf) - 1);
    assert(n == (zip_int64_t)payload_len);
    assert(memcmp(buf, payload, payload_len) == 0);

    zip_fclose(zf);
    zip_close(za);
    unlink(path);

    printf("libzip test OK: %s\n", v);
    return 0;
}
