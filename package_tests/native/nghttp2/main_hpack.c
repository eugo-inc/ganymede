/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp2: hpack; calls nghttp2_hd_deflate_new(), nghttp2_hd_inflate_new(), nghttp2_hd_deflate_bound(), nghttp2_hd_deflate_hd().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp2/nghttp2.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    nghttp2_hd_deflater *deflater = NULL;
    nghttp2_hd_inflater *inflater = NULL;

    int rv = nghttp2_hd_deflate_new(&deflater, NGHTTP2_DEFAULT_HEADER_TABLE_SIZE);
    assert(rv == 0);
    assert(deflater != NULL);

    rv = nghttp2_hd_inflate_new(&inflater);
    assert(rv == 0);
    assert(inflater != NULL);

    const char name[]  = "x-demo-probe";
    const char value[] = "nghttp2-hpack-roundtrip";
    nghttp2_nv nv = {
        .name      = (uint8_t *)name,
        .value     = (uint8_t *)value,
        .namelen   = sizeof(name) - 1,
        .valuelen  = sizeof(value) - 1,
        .flags     = NGHTTP2_NV_FLAG_NONE,
    };

    size_t bound = nghttp2_hd_deflate_bound(deflater, &nv, 1);
    assert(bound > 0);

    uint8_t *buf = (uint8_t *)malloc(bound);
    assert(buf != NULL);

    ssize_t encoded = nghttp2_hd_deflate_hd(deflater, buf, bound, &nv, 1);
    assert(encoded > 0);
    assert((size_t)encoded <= bound);

    /* Now decode the encoded block and assert byte-identical recovery. */
    nghttp2_nv decoded = {0};
    int inflate_flags = 0;
    ssize_t consumed = nghttp2_hd_inflate_hd2(
        inflater, &decoded, &inflate_flags, buf, (size_t)encoded, 1);
    assert(consumed >= 0);
    assert(inflate_flags & NGHTTP2_HD_INFLATE_EMIT);
    assert(decoded.namelen == nv.namelen);
    assert(decoded.valuelen == nv.valuelen);
    assert(memcmp(decoded.name, nv.name, nv.namelen) == 0);
    assert(memcmp(decoded.value, nv.value, nv.valuelen) == 0);

    /* Drain remaining bytes (none expected) and signal final block. */
    consumed = nghttp2_hd_inflate_hd2(
        inflater, &decoded, &inflate_flags, buf + consumed,
        (size_t)encoded - (size_t)consumed, 1);
    assert(consumed >= 0);
    assert(inflate_flags & NGHTTP2_HD_INFLATE_FINAL);

    free(buf);
    nghttp2_hd_inflate_del(inflater);
    nghttp2_hd_deflate_del(deflater);

    printf("nghttp2 HPACK round-trip OK\n");
    return 0;
}
