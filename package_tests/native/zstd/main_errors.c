/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for zstd: errors; calls ZSTD_decompress(), ZSTD_isError(), ZSTD_getErrorName(), ZSTD_getErrorCode().
 */

#include <zstd.h>
#include <zstd_errors.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Garbage frame: not a valid zstd magic number, decompress must reject. */
    static const char garbage[] = "this is definitely not a zstd frame, just ascii bytes";
    char dbuf[64];
    size_t rc = ZSTD_decompress(dbuf, sizeof(dbuf), garbage, sizeof(garbage) - 1);
    assert(ZSTD_isError(rc));

    /* ZSTD_getErrorName must return a non-empty, NUL-terminated string. */
    const char *name = ZSTD_getErrorName(rc);
    assert(name != NULL);
    assert(strlen(name) > 0);

    /* ZSTD_getErrorCode maps the size_t to the ZSTD_ErrorCode enum;
     * for a magic-number failure the canonical code is prefix_unknown. */
    ZSTD_ErrorCode code = ZSTD_getErrorCode(rc);
    assert(code != ZSTD_error_no_error);
    assert(code == ZSTD_error_prefix_unknown);

    /* ZSTD_getErrorString (enum -> string) must also yield a non-empty string. */
    const char *enum_name = ZSTD_getErrorString(code);
    assert(enum_name != NULL);
    assert(strlen(enum_name) > 0);

    /* Sanity: an "ok" size_t (0) must NOT be classified as an error,
     * and its error-name lookup is still safe. */
    assert(!ZSTD_isError(0));
    const char *ok_name = ZSTD_getErrorName(0);
    assert(ok_name != NULL);

    unsigned ver = ZSTD_versionNumber();
    assert(ver / 10000 >= 1);

    printf("zstd errors OK: code=%d name=\"%s\" enum_name=\"%s\" version=%u\n",
           (int)code, name, enum_name, ver);
    return 0;
}
