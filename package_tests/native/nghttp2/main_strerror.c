/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp2: strerror; calls nghttp2_strerror(), nghttp2_http2_strerror().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *lib_err = nghttp2_strerror(NGHTTP2_ERR_INVALID_ARGUMENT);
    assert(lib_err != NULL);
    assert(strlen(lib_err) > 0);

    const char *wire_err = nghttp2_http2_strerror(NGHTTP2_PROTOCOL_ERROR);
    assert(wire_err != NULL);
    assert(strlen(wire_err) > 0);

    /* The two namespaces share neither encoding nor messages, so their
     * canonical strings for distinct codes must not be identical. */
    assert(strcmp(lib_err, wire_err) != 0);

    /* Unknown code must still resolve to a non-NULL fallback string so
     * naive `printf("%s", ...)` callers don't segfault. */
    const char *unknown = nghttp2_strerror(999999);
    assert(unknown != NULL);
    assert(strlen(unknown) > 0);

    printf("nghttp2 strerror OK: lib=\"%s\" wire=\"%s\"\n", lib_err, wire_err);
    return 0;
}
