/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for jbigkit: jbig85; calls jbg85_enc_init(), jbg85_enc_lineout(), jbg85_dec_init(), jbg85_dec_in().
 *
 * Links against libjbig85, the separate T.85 library that upstream jbigkit builds next to
 * libjbig. Debian and Ubuntu put the jbg85_* functions into libjbig and ship no libjbig85,
 * so there this program does not link (-ljbig85 not found) and the libjbig85 row fails.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig85.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IMG_W 16
#define IMG_H 8
#define BPL   ((IMG_W + 7) / 8)

/* 16x8 1-bpp test pattern: alternating-bit rows + a couple of edge rows. */
static const unsigned char src_pixels[IMG_H][BPL] = {
    {0xFF, 0xFF},
    {0xAA, 0xAA},
    {0x55, 0x55},
    {0x00, 0x00},
    {0xF0, 0x0F},
    {0x0F, 0xF0},
    {0xCC, 0x33},
    {0x33, 0xCC},
};

/* Encoded-output sink for jbg85_enc. */
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

/* Decoded line sink: capture each decoded line in row-order. */
struct line_collector {
    unsigned char rows[IMG_H][BPL];
    unsigned long max_y_seen;
    int rows_received;
};

static int line_capture(const struct jbg85_dec_state *s,
                        unsigned char *start, size_t len,
                        unsigned long y, void *file) {
    (void)s;
    struct line_collector *c = (struct line_collector *)file;
    assert(y < IMG_H);
    assert(len == BPL);
    memcpy(c->rows[y], start, len);
    if (y > c->max_y_seen) c->max_y_seen = y;
    c->rows_received++;
    return 0; /* 0 = continue */
}

int main(void) {
    /* --- ENCODE (line-by-line via jbg85_enc_lineout) --- */
    struct sink out = {0};
    out.cap = 128;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    struct jbg85_enc_state enc;
    jbg85_enc_init(&enc, IMG_W, IMG_H, sink_write, &out);
    /* Defaults: produce a single-stripe, full-length BIE. */

    /* Feed lines with the documented rolling 3-line history. */
    unsigned char *prev = NULL;
    unsigned char *prevprev = NULL;
    for (int y = 0; y < IMG_H; y++) {
        /* Cast away const — library takes non-const but does not modify. */
        unsigned char *cur = (unsigned char *)src_pixels[y];
        jbg85_enc_lineout(&enc, cur, prev, prevprev);
        prevprev = prev;
        prev = cur;
    }
    assert(out.len > 20);  /* BIE header alone is 20 bytes. */

    /* --- DECODE (streaming, line_out callback) --- */
    struct line_collector got = {{{0}}, 0, 0};
    /* jbg85 decoder requires a caller-provided 3-line history buffer. */
    unsigned char linebuf[3 * BPL];
    struct jbg85_dec_state dec;
    jbg85_dec_init(&dec, linebuf, sizeof(linebuf), line_capture, &got);

    size_t cnt = 0;
    int rc = jbg85_dec_in(&dec, out.data, out.len, &cnt);
    /* The T.85 encoder defaults to VLENGTH (variable-length, options==0x48 =
     * JBG_VLENGTH|JBG_TPBON), so after consuming the entire in-memory BIE the
     * streaming decoder cannot know the stream has ended and returns JBG_EAGAIN
     * ("need more data"). True completion is confirmed below by
     * jbg85_dec_end() == JBG_EOK. JBG_EOK / JBG_EOK_INTR are the
     * fixed-length / interrupted variants. All three are valid here. */
    assert(rc == JBG_EOK || rc == JBG_EOK_INTR || rc == JBG_EAGAIN);
    assert(cnt <= out.len);

    int end_rc = jbg85_dec_end(&dec);
    assert(end_rc == JBG_EOK);

    /* Width/height macros must report the BIH-encoded dims. */
    assert(jbg85_dec_getwidth(&dec) == IMG_W);
    assert(jbg85_dec_getheight(&dec) == IMG_H);
    assert(jbg85_dec_validwidth(&dec));

    /* Every row was delivered AND each row matches the source. */
    assert(got.rows_received == IMG_H);
    assert(got.max_y_seen == IMG_H - 1);
    for (int y = 0; y < IMG_H; y++) {
        assert(memcmp(got.rows[y], src_pixels[y], BPL) == 0);
    }

    /* jbg85_strerror must resolve a known error code. */
    const char *msg = jbg85_strerror(JBG_EOK);
    assert(msg != NULL);
    assert(msg[0] != '\0');

    free(out.data);
    printf("jbig85 test OK: %zu bytes BIE, %dx%d streamed round-trip match\n",
           out.len, IMG_W, IMG_H);
    return 0;
}
