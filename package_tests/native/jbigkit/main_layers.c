/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jbigkit: layers; calls jbg_ceil_half(), jbg_enc_init(), jbg_enc_layers(), jbg_enc_lrange().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jbig.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define IMG_W 32
#define IMG_H 32
#define BPL   ((IMG_W + 7) / 8)
#define NLAYERS 2  /* Encode 3 resolution layers total (layer 0 + 2 lower). */

static unsigned char src_pixels[IMG_H * BPL];
/* The multi-layer encoder reuses the caller's highest-resolution plane buffer
 * while building the resolution-reduction pyramid, so src_pixels is mutated
 * in place by jbg_enc_out(). Keep a pristine snapshot to compare against. */
static unsigned char src_orig[IMG_H * BPL];

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
    /* Deterministic source: alternating bands so resolution-reduction has
     * something non-trivial to encode at each layer. */
    for (int y = 0; y < IMG_H; y++) {
        for (int b = 0; b < BPL; b++) {
            src_pixels[y * BPL + b] = (unsigned char)((y & 1) ? 0xAA : 0x55);
        }
    }
    memcpy(src_orig, src_pixels, sizeof(src_pixels));

    /* Sanity: jbg_ceil_half models the per-layer resolution ladder. */
    /* Layer 0 = 32x32; layer 1 = 16x16; layer 2 = 8x8. */
    assert(jbg_ceil_half(IMG_W, 1) == 16);
    assert(jbg_ceil_half(IMG_W, 2) == 8);
    assert(jbg_ceil_half(IMG_H, 1) == 16);
    assert(jbg_ceil_half(IMG_H, 2) == 8);

    /* --- ENCODE with NLAYERS lower-resolution layers. --- */
    struct sink out = {0};
    out.cap = 512;
    out.data = (unsigned char *)malloc(out.cap);
    assert(out.data != NULL);

    unsigned char *planes[1] = { src_pixels };
    struct jbg_enc_state enc;
    jbg_enc_init(&enc, IMG_W, IMG_H, 1, planes, sink_write, &out);

    jbg_enc_layers(&enc, NLAYERS);

    /* Encode all NLAYERS+1 layers (range [0, NLAYERS] inclusive).
     * jbg_enc_lrange() returns the number of resolution layers d set by
     * jbg_enc_layers(), i.e. NLAYERS here, NOT a 0/non-zero status code. */
    int lr = jbg_enc_lrange(&enc, 0, NLAYERS);
    assert(lr == NLAYERS);

    jbg_enc_out(&enc);
    jbg_enc_free(&enc);
    /* A progressive BIE has multiple stripes; it should be appreciably larger
     * than a bare BIH but not absurdly so given the regular pattern. */
    assert(out.len > 20);

    /* --- DECODE: feed the whole multi-layer BIE in one go. --- */
    struct jbg_dec_state dec;
    jbg_dec_init(&dec);
    size_t cnt = 0;
    int rc = jbg_dec_in(&dec, out.data, out.len, &cnt);
    assert(rc == JBG_EOK);
    assert(cnt == out.len);

    /* Decoder reports the FULL-resolution dimensions for the top layer. */
    assert(jbg_dec_getwidth(&dec) == IMG_W);
    assert(jbg_dec_getheight(&dec) == IMG_H);

    int max_dl = dec.d;
    assert(max_dl == NLAYERS);
    assert(dec.dl == 0);

    unsigned char *decoded = jbg_dec_getimage(&dec, 0);
    assert(decoded != NULL);
    assert(memcmp(decoded, src_orig, sizeof(src_orig)) == 0);

    /* getsize is the per-plane byte count at the CURRENT resolution. */
    unsigned long sz = jbg_dec_getsize(&dec);
    assert(sz == (unsigned long)BPL * IMG_H);

    jbg_dec_free(&dec);
    free(out.data);

    printf("jbigkit layers test OK: %d-layer BIE = %zu bytes, max_dl=%d, "
           "%lu bytes/plane at full res\n",
           NLAYERS + 1, out.len, max_dl, sz);
    return 0;
}
