/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jbigkit: core API; calls jbg_enc_init(), jbg_enc_out(), jbg_enc_free(), jbg_dec_init().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 8x8 1-bpp bitmap (one byte per row, MSB = leftmost pixel): a diagonal pattern. */
#define IMG_W 8
#define IMG_H 8
static unsigned char src_pixels[IMG_H] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01,
};

/* Encoded-output sink: append into a heap buffer. */
struct sink {
    unsigned char *data;
    size_t len;
    size_t cap;
};

static void sink_write(unsigned char *start, size_t len, void *file) {
    struct sink *s = (struct sink *)file;
    if (s->len + len > s->cap) {
        s->cap = (s->len + len) * 2;
        s->data = (unsigned char *)realloc(s->data, s->cap);
        assert(s->data != NULL);
    }
    memcpy(s->data + s->len, start, len);
    s->len += len;
}

int main(void) {
    /* --- ENCODE --- */
    struct sink out = {0};
    out.cap = 256;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *planes[1] = { src_pixels };
    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, 1, planes, sink_write, &out);
    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    assert(out.len > 0);

    /* --- DECODE --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);
    size_t cnt = 0;
    int rc = jbg_dec_in(&dec, out.data, out.len, &cnt);
    assert(rc == JBG_EOK);
    assert(cnt == out.len);

    assert(jbg_dec_getwidth(&dec) == IMG_W);
    assert(jbg_dec_getheight(&dec) == IMG_H);

    unsigned char *decoded = jbg_dec_getimage(&dec, 0);
    assert(decoded != NULL);
    assert(memcmp(decoded, src_pixels, sizeof(src_pixels)) == 0);

    jbg_dec_free(&dec);
    free(out.data);

    /* jbg_strerror must resolve for JBG_EOK. */
    const char *msg = jbg_strerror(JBG_EOK);
    assert(msg != NULL);

    printf("jbigkit test OK: encoded %zu bytes, round-trip %dx%d match\n",
           out.len, IMG_W, IMG_H);
    return 0;
}
