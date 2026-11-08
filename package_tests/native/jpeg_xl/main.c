/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jpeg_xl: core API; calls JxlDecoderVersion(), JxlEncoderVersion(), JxlDecoderCreate(), JxlDecoderDestroy().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <jxl/decode.h>
#include <jxl/encode.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Decoder + encoder version must be a non-zero matched value (same libjxl). */
    uint32_t dec_ver = JxlDecoderVersion();
    uint32_t enc_ver = JxlEncoderVersion();
    assert(dec_ver != 0);
    assert(enc_ver != 0);
    assert(dec_ver == enc_ver);

    JxlDecoder *dec = JxlDecoderCreate(NULL);
    assert(dec != NULL);
    JxlDecoderDestroy(dec);

    JxlEncoder *enc = JxlEncoderCreate(NULL);
    assert(enc != NULL);
    JxlEncoderDestroy(enc);

    printf("jpeg_xl test OK: libjxl version %u\n", dec_ver);
    return 0;
}
