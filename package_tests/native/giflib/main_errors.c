/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for giflib: errors; calls GifErrorString(), EGifOpen(), EGifPutLine(), EGifCloseFile().
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
} MemBuf;

static int write_cb(GifFileType *gif, const GifByteType *buf, int n) {
    MemBuf *m = (MemBuf *)gif->UserData;
    if (m->len + (size_t)n > m->cap) return 0;
    memcpy(m->data + m->len, buf, (size_t)n);
    m->len += (size_t)n;
    return n;
}

/* Walk one documented, CONTIGUOUS error-code range; every code in it must
 * translate to a non-empty string. Returns the number of codes that did
 * not, having named each on stderr. */
static int count_untranslatable(const char *which, int lowest, int highest) {
    int failures = 0;
    for (int code = lowest; code <= highest; code++) {
        const char *msg = GifErrorString(code);
        if (msg == NULL || msg[0] == '\0') {
            fprintf(stderr, "giflib: %s error code %d has no message (GifErrorString returned %s)\n",
                    which, code, msg == NULL ? "NULL" : "an empty string");
            failures++;
        }
    }
    return failures;
}

int main(void) {
    /* The error-message table, both halves. The ranges come from gif_lib.h
     * itself, so this says "every code THIS header documents", not "every
     * code some past version numbered". */
    int failures = 0;
    failures += count_untranslatable("decoder", D_GIF_ERR_OPEN_FAILED, D_GIF_ERR_EOF_TOO_SOON);
    failures += count_untranslatable("encoder", E_GIF_ERR_OPEN_FAILED, E_GIF_ERR_NOT_WRITEABLE);
    if (failures != 0) {
        fprintf(stderr, "giflib errors test FAILED: %d documented error code(s) have no message\n", failures);
        return 1;
    }

    /* Out-of-range codes (negative / huge) must NOT crash — giflib
     * documents NULL return for unknown codes. */
    const char *unknown = GifErrorString(999999);
    /* Either NULL or a non-empty string is acceptable; what's NOT
     * acceptable is undefined behavior, which a crash here would show. */
    (void)unknown;
    const char *negative = GifErrorString(-1);
    (void)negative;

    unsigned char buf[4096];
    MemBuf out = { buf, 0, sizeof(buf) };
    int err = 0;
    GifFileType *enc = EGifOpen(&out, write_cb, &err);
    assert(enc != NULL);

    GifByteType pixel = 0;
    int put_result = EGifPutLine(enc, &pixel, 1);
    assert(put_result == GIF_ERROR);
    assert(enc->Error != 0);
    if (enc->Error < E_GIF_ERR_OPEN_FAILED || enc->Error > E_GIF_ERR_NOT_WRITEABLE) {
        fprintf(stderr, "giflib errors test FAILED: writer error %d is outside the encoder range %d..%d\n",
                enc->Error, E_GIF_ERR_OPEN_FAILED, E_GIF_ERR_NOT_WRITEABLE);
        return 1;
    }
    const char *err_msg = GifErrorString(enc->Error);
    assert(err_msg != NULL);
    assert(err_msg[0] != '\0');

    /* Closing a writer that's in an error state may itself return
     * GIF_ERROR — accept either outcome; the goal is to free the
     * handle without leaking. */
    (void)EGifCloseFile(enc, &err);

    printf("giflib errors test OK\n");
    return 0;
}
