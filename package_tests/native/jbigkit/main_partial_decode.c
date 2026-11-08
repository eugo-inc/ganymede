/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jbigkit: partial decode; calls jbg_enc_init(), jbg_enc_out(), jbg_enc_free(), jbg_dec_init().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IMG_W 16
#define IMG_H 16

static unsigned char src_pixels[IMG_H * 2];

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
    /* Deterministic source pattern. */
    for (int i = 0; i < (int)sizeof(src_pixels); i++) {
        src_pixels[i] = (unsigned char)(0xA5u ^ (i * 17u));
    }

    struct sink out = {0};
    out.cap = 256;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *planes[1] = { src_pixels };
    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, 1, planes, sink_write, &out);
    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    /* BIH alone is 20 bytes; need strictly more to have something to chunk past it. */
    assert(out.len > 20);

    /* --- DECODE in 1-byte chunks. --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);

    int saw_eagain = 0;
    int final_rc = JBG_EAGAIN;
    size_t total_consumed = 0;

    for (size_t i = 0; i < out.len; i++) {
        size_t cnt = 0;
        int rc = jbg_dec_in(&dec, out.data + i, 1, &cnt);
        total_consumed += cnt;
        if (i + 1 < out.len) {
            assert(rc == JBG_EAGAIN);
            saw_eagain = 1;
        } else {
            final_rc = rc;
        }
    }
    assert(saw_eagain);
    assert(total_consumed == out.len);
    assert(final_rc == JBG_EOK || final_rc == JBG_EAGAIN);

    /* Geometry must be readable as soon as the BIH (first 20 bytes) was consumed. */
    assert(jbg_dec_getwidth(&dec) == IMG_W);
    assert(jbg_dec_getheight(&dec) == IMG_H);

    unsigned char *decoded = jbg_dec_getimage(&dec, 0);
    assert(decoded != NULL);
    assert(memcmp(decoded, src_pixels, sizeof(src_pixels)) == 0);

    jbg_dec_free(&dec);

    /* --- jbg_strerror: distinct, non-null, non-empty messages per code. --- */
    const char *m_ok      = jbg_strerror(JBG_EOK);
    const char *m_again   = jbg_strerror(JBG_EAGAIN);
    const char *m_nomem   = jbg_strerror(JBG_ENOMEM);
    const char *m_inval   = jbg_strerror(JBG_EINVAL);
    assert(m_ok    != NULL && m_ok[0]    != '\0');
    assert(m_again != NULL && m_again[0] != '\0');
    assert(m_nomem != NULL && m_nomem[0] != '\0');
    assert(m_inval != NULL && m_inval[0] != '\0');
    /* The OK message must NOT be identical to the EINVAL message — catches a
     * broken table that returns the same string for every code. */
    assert(strcmp(m_ok, m_inval) != 0);
    assert(strcmp(m_again, m_inval) != 0);

    free(out.data);

    printf("jbigkit partial-decode test OK: BIE=%zu bytes fed 1-by-1, "
           "saw EAGAIN, final rc=0x%02x, strerror table distinct\n",
           out.len, (unsigned)final_rc);
    return 0;
}
