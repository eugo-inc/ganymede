/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for giflib: core API; calls EGifOpen(), GifMakeMapObject(), EGifPutScreenDesc(), EGifPutImageDesc().
 */

#include <gif_lib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Bounded scratch buffer used by the in-memory output + input callbacks. */
typedef struct {
    unsigned char *data;
    size_t len;
    size_t cap;
    size_t read_pos;
} MemBuf;

static int write_cb(GifFileType *gif, const GifByteType *buf, int n) {
    MemBuf *m = (MemBuf *)gif->UserData;
    assert(m->len + (size_t)n <= m->cap);
    memcpy(m->data + m->len, buf, (size_t)n);
    m->len += (size_t)n;
    return n;
}

static int read_cb(GifFileType *gif, GifByteType *buf, int n) {
    MemBuf *m = (MemBuf *)gif->UserData;
    size_t remaining = m->len - m->read_pos;
    size_t take = (size_t)n < remaining ? (size_t)n : remaining;
    memcpy(buf, m->data + m->read_pos, take);
    m->read_pos += take;
    return (int)take;
}

int main(void) {
    assert(GIFLIB_MAJOR >= 5);

    unsigned char buf[4096];
    MemBuf out = { buf, 0, sizeof(buf), 0 };

    int err = 0;
    GifFileType *enc = EGifOpen(&out, write_cb, &err);
    assert(enc != NULL);

    GifColorType colors[2] = { {0, 0, 0}, {255, 255, 255} };
    ColorMapObject *cmap = GifMakeMapObject(2, colors);
    assert(cmap != NULL);

    assert(EGifPutScreenDesc(enc, 1, 1, 1, 0, cmap) == GIF_OK);
    assert(EGifPutImageDesc(enc, 0, 0, 1, 1, false, NULL) == GIF_OK);
    GifByteType pixel = 1;
    assert(EGifPutLine(enc, &pixel, 1) == GIF_OK);
    assert(EGifCloseFile(enc, &err) == GIF_OK);
    GifFreeMapObject(cmap);

    assert(out.len > 0);

    MemBuf in = { buf, out.len, out.len, 0 };
    GifFileType *dec = DGifOpen(&in, read_cb, &err);
    assert(dec != NULL);
    assert(DGifSlurp(dec) == GIF_OK);
    assert(dec->SWidth == 1);
    assert(dec->SHeight == 1);
    assert(dec->ImageCount == 1);
    assert(DGifCloseFile(dec, &err) == GIF_OK);

    printf("giflib test OK: %d.%d.%d\n", GIFLIB_MAJOR, GIFLIB_MINOR, GIFLIB_RELEASE);
    return 0;
}
