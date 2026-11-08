/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp2: core API; calls nghttp2_version(), nghttp2_session_callbacks_new(), nghttp2_session_callbacks_del().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version metadata must resolve and report a non-empty proto id ("h2"). */
    nghttp2_info *info = nghttp2_version(0);
    assert(info != NULL);
    assert(info->version_str != NULL);
    assert(info->proto_str != NULL);
    assert(strlen(info->proto_str) > 0);

    nghttp2_session_callbacks *cb = NULL;
    int rv = nghttp2_session_callbacks_new(&cb);
    assert(rv == 0);
    assert(cb != NULL);
    nghttp2_session_callbacks_del(cb);

    printf("nghttp2 test OK: version=%s proto=%s\n", info->version_str, info->proto_str);
    return 0;
}
