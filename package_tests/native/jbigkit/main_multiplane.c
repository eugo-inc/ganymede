/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jbigkit: multiplane; calls jbg_enc_init(), jbg_enc_out(), jbg_enc_free(), jbg_dec_init().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IMG_W 16
#define IMG_H 8
#define BPL   ((IMG_W + 7) / 8)
#define NPLANES 2  /* 2 bitplanes => 4 grey levels per pixel */

/* Two independent bitplanes — distinct patterns so a stride bug (swapping the
 * two plane pointers) would surface as a memcmp mismatch. */
static unsigned char plane0[IMG_H * BPL];
static unsigned char plane1[IMG_H * BPL];

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
    /* Deterministic, distinct per-plane patterns. */
    for (int i = 0; i < (int)sizeof(plane0); i++) {
        plane0[i] = (unsigned char)(0xA5u ^ (i * 13u));
        plane1[i] = (unsigned char)(0x5Au ^ (i * 29u));
    }

    /* --- ENCODE: 2 distinct bitplanes. --- */
    struct sink out = {0};
    out.cap = 256;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *planes[NPLANES] = { plane0, plane1 };
    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, NPLANES, planes, sink_write, &out);
    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    assert(out.len > 20);

    /* --- DECODE & verify per-plane content. --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);
    size_t cnt = 0;
    int rc = jbg_dec_in(&dec, out.data, out.len, &cnt);
    assert(rc == JBG_EOK);
    assert(cnt == out.len);

    assert(jbg_dec_getwidth(&dec) == IMG_W);
    assert(jbg_dec_getheight(&dec) == IMG_H);
    assert(jbg_dec_getplanes(&dec) == NPLANES);

    unsigned char *decoded0 = jbg_dec_getimage(&dec, 0);
    unsigned char *decoded1 = jbg_dec_getimage(&dec, 1);
    assert(decoded0 != NULL);
    assert(decoded1 != NULL);
    assert(decoded0 != decoded1);
    assert(memcmp(decoded0, plane0, sizeof(plane0)) == 0);
    assert(memcmp(decoded1, plane1, sizeof(plane1)) == 0);
    assert(memcmp(decoded0, decoded1, sizeof(plane0)) != 0);  /* distinct content */

    /* getsize is per-plane; getsize_merged is planes * per-plane bytes. */
    unsigned long sz = jbg_dec_getsize(&dec);
    unsigned long sz_merged = jbg_dec_getsize_merged(&dec);
    assert(sz == (unsigned long)BPL * IMG_H);
    /* Merged size for multi-plane: 1 byte/pixel (grey-encoded) at this plane count. */
    assert(sz_merged >= sz);  /* at minimum same; for >1 plane usually larger */

    jbg_dec_free(&dec);
    free(out.data);

    printf("jbigkit multiplane test OK: %d planes, %zu bytes BIE, %lu bytes/plane\n",
           NPLANES, out.len, sz);
    return 0;
}
