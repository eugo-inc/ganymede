/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jbigkit: options; calls jbg_ceil_half(), jbg_enc_init(), jbg_enc_options(), jbg_enc_lrange().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IMG_W 32
#define IMG_H 16

/* 32x16 1-bpp test pattern — 4 bytes/row, 16 rows. */
static unsigned char src_pixels[IMG_H * 4];

struct sink {
    unsigned char *data;
    size_t len;
    size_t cap;
};

static void sink_write(unsigned char *start, size_t len, void *file) {
    struct sink *s = (struct sink *)file;
    if (s->len + len > s->cap) {
        s->cap = (s->len + len) * 2 + 64;
        s->data = (unsigned char *)realloc(s->data, s->cap);
        assert(s->data != NULL);
    }
    memcpy(s->data + s->len, start, len);
    s->len += len;
}

int main(void) {
    /* Fill source with a deterministic pattern. */
    for (int i = 0; i < (int)sizeof(src_pixels); i++) {
        src_pixels[i] = (unsigned char)((i * 37u + 11u) & 0xFFu);
    }

    /* --- jbg_ceil_half: pure utility, no state. --- */
    assert(jbg_ceil_half(32, 0) == 32);
    assert(jbg_ceil_half(32, 1) == 16);
    assert(jbg_ceil_half(33, 1) == 17);  /* ceiling, not floor */
    assert(jbg_ceil_half(1, 3) == 1);    /* never collapses below 1 */

    /* --- ENCODE with explicit options + layers --- */
    struct sink out = {0};
    out.cap = 256;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *planes[1] = { src_pixels };
    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, 1, planes, sink_write, &out);

    /* Force a known config: order=SEQ, options=TPBON, l0=4 lines/stripe, mx=8, my=0. */
    jbg_enc_options(&enc, JBG_SEQ, JBG_TPBON, 4, 8, 0);

    /* Layer range: encode only layer 0 (full resolution). jbg_enc_lrange() returns the
     * number of resolution layers d, not a status code: 0 here, as jbg_enc_layers() has
     * not been called yet. */
    int lr = jbg_enc_lrange(&enc, 0, 0);
    assert(lr == 0);

    /* Single-layer (no progressive resolutions). */
    jbg_enc_layers(&enc, 0);

    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    assert(out.len > 20);

    /* --- jbg_newlen on the BIE: must accept a valid header. --- */
    int nl = jbg_newlen(out.data, out.len);
    assert(nl == JBG_EOK);

    /* --- DECODE + verify the metadata-query API surface. --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);
    size_t cnt = 0;
    int rc = jbg_dec_in(&dec, out.data, out.len, &cnt);
    assert(rc == JBG_EOK);
    assert(cnt == out.len);

    assert(jbg_dec_getwidth(&dec) == IMG_W);
    assert(jbg_dec_getheight(&dec) == IMG_H);

    /* getsize: bytes for a SINGLE plane bitmap. 32 px / 8 = 4 bpl * 16 rows = 64. */
    unsigned long sz = jbg_dec_getsize(&dec);
    assert(sz == (unsigned long)(IMG_W / 8) * IMG_H);
    assert(sz == sizeof(src_pixels));

    unsigned long sz_merged = jbg_dec_getsize_merged(&dec);
    assert(sz_merged == (unsigned long)IMG_W * IMG_H * 1);
    assert(sz_merged == sz * 8);

    /* The decoded bitmap must match the source byte-for-byte. */
    unsigned char *decoded = jbg_dec_getimage(&dec, 0);
    assert(decoded != NULL);
    assert(memcmp(decoded, src_pixels, sizeof(src_pixels)) == 0);

    /* jbg_strerror must resolve for a non-OK code too. */
    const char *msg_inval = jbg_strerror(JBG_EINVAL);
    assert(msg_inval != NULL);
    assert(msg_inval[0] != '\0');

    jbg_dec_free(&dec);
    free(out.data);

    printf("jbigkit options test OK: %lu bytes/plane, jbg_newlen + getsize verified\n",
           sz);
    return 0;
}
