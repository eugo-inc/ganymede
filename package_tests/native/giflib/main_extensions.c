/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for giflib: extensions; calls EGifOpen(), GifMakeMapObject(), EGifPutScreenDesc(), EGifPutExtensionLeader().
 */

#include <gif_lib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    unsigned char buf[8192];
    MemBuf out = { buf, 0, sizeof(buf), 0 };

    int err = 0;
    GifFileType *enc = EGifOpen(&out, write_cb, &err);
    assert(enc != NULL);

    GifColorType colors[2] = { {0, 0, 0}, {255, 255, 255} };
    ColorMapObject *cmap = GifMakeMapObject(2, colors);
    assert(cmap != NULL);

    assert(EGifPutScreenDesc(enc, 1, 1, 1, 0, cmap) == GIF_OK);

    /* Application Extension: NETSCAPE2.0 (the loop-count signaling block). */
    const char netscape_id[11] = { 'N','E','T','S','C','A','P','E','2','.','0' };
    assert(EGifPutExtensionLeader(enc, APPLICATION_EXT_FUNC_CODE) == GIF_OK);
    assert(EGifPutExtensionBlock(enc, sizeof(netscape_id), netscape_id) == GIF_OK);
    /* Sub-block: 0x01 (loop sub-block id), 16-bit loop count = 0 (infinite). */
    const unsigned char loop_sub[3] = { 0x01, 0x00, 0x00 };
    assert(EGifPutExtensionBlock(enc, sizeof(loop_sub), loop_sub) == GIF_OK);
    assert(EGifPutExtensionTrailer(enc) == GIF_OK);

    /* Comment Extension — single block. */
    const char comment[] = "demo-test";
    assert(EGifPutExtension(enc, COMMENT_EXT_FUNC_CODE, (int)(sizeof(comment) - 1), comment) == GIF_OK);

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
    assert(dec->ImageCount == 1);

    /* Comment + Application extension may be attached either to the GIF
     * (ExtensionBlocks on the GifFileType) or to the SavedImage. Look in
     * both spots, count APPLICATION + COMMENT codes; both must be found. */
    int saw_application = 0, saw_comment = 0;
    for (int i = 0; i < dec->ExtensionBlockCount; i++) {
        int fn = dec->ExtensionBlocks[i].Function;
        if (fn == APPLICATION_EXT_FUNC_CODE) saw_application = 1;
        if (fn == COMMENT_EXT_FUNC_CODE)     saw_comment = 1;
    }
    SavedImage *si = &dec->SavedImages[0];
    for (int i = 0; i < si->ExtensionBlockCount; i++) {
        int fn = si->ExtensionBlocks[i].Function;
        if (fn == APPLICATION_EXT_FUNC_CODE) saw_application = 1;
        if (fn == COMMENT_EXT_FUNC_CODE)     saw_comment = 1;
    }
    assert(saw_application);
    assert(saw_comment);

    assert(DGifCloseFile(dec, &err) == GIF_OK);
    printf("giflib extensions test OK\n");
    return 0;
}
