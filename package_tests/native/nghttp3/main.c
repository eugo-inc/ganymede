/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp3: core API; calls nghttp3_version(), nghttp3_strerror().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const nghttp3_info *info = nghttp3_version(0);
    assert(info != NULL);
    assert(info->version_str != NULL);
    assert(strlen(info->version_str) > 0);

    /* NGHTTP3_ERR_INVALID_ARGUMENT is part of nghttp3's stable public error enum.
     * strerror must return a non-empty string for it. */
    const char *msg = nghttp3_strerror(NGHTTP3_ERR_INVALID_ARGUMENT);
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    printf("nghttp3 test OK: %s\n", info->version_str);
    return 0;
}
