/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jbigkit: split merge; calls jbg_split_planes(), jbg_enc_init(), jbg_enc_out(), jbg_enc_free().
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
#define BPP   2                     /* bits-per-pixel; 4 grey levels */
#define NPLANES BPP
#define BPL   ((IMG_W + 7) / 8)     /* bytes per bitplane row */
#define PIXELS (IMG_W * IMG_H)

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
    unsigned char grey[PIXELS];
    for (int i = 0; i < PIXELS; i++) {
        grey[i] = (unsigned char)(i & ((1 << BPP) - 1));  /* 0..3 cycle */
    }

    /* --- SPLIT: byte-per-pixel -> NPLANES separate bitplanes. --- */
    unsigned char plane_buf[NPLANES][IMG_H * BPL];
    unsigned char *split_dst[NPLANES];
    for (int p = 0; p < NPLANES; p++) {
        memset(plane_buf[p], 0, IMG_H * BPL);
        split_dst[p] = plane_buf[p];
    }

    jbg_split_planes(IMG_W, IMG_H, BPP, NPLANES, grey, split_dst, 1);

    struct sink out = {0};
    out.cap = 256;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *enc_planes[NPLANES];
    for (int p = 0; p < NPLANES; p++) enc_planes[p] = plane_buf[p];

    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, NPLANES, enc_planes, sink_write, &out);
    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    assert(out.len > 20);

    /* --- DECODE the BIE back. --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);
    size_t cnt = 0;
    int rc = jbg_dec_in(&dec, out.data, out.len, &cnt);
    assert(rc == JBG_EOK);
    assert(cnt == out.len);
    assert(jbg_dec_getplanes(&dec) == NPLANES);

    /* --- MERGE: bitplanes -> byte-per-pixel grey, then verify it matches. --- */
    unsigned char merged_grey[PIXELS];
    memset(merged_grey, 0xFF, sizeof(merged_grey));  /* pre-poison; must be overwritten */

    /* jbg_dec_merge_planes signature mirrors split:
     *   jbg_dec_merge_planes(const struct jbg_dec_state *s, int use_graycode,
     *                        void (*data_out)(unsigned char *, size_t, void *),
     *                        void *file);
     * Writes one merged byte per pixel via the data_out callback. */
    {
        struct sink merge_sink = {0};
        merge_sink.cap = PIXELS + 16;
        merge_sink.data = (unsigned char *)malloc(merge_sink.cap);
        assert(merge_sink.data != NULL);

        jbg_dec_merge_planes(&dec, 1, sink_write, &merge_sink);
        assert(merge_sink.len == PIXELS);
        memcpy(merged_grey, merge_sink.data, PIXELS);
        free(merge_sink.data);
    }

    assert(memcmp(merged_grey, grey, PIXELS) == 0);

    jbg_dec_free(&dec);
    free(out.data);

    printf("jbigkit split/merge test OK: %d-bpp grey, %d planes, "
           "%zu BIE bytes, %d-pixel round-trip lossless\n",
           BPP, NPLANES, out.len, PIXELS);
    return 0;
}
