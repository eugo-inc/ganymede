/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for nghttp3: qpack; calls nghttp3_mem_default(), nghttp3_qpack_encoder_new(), nghttp3_qpack_encoder_get_num_blocked_streams(), nghttp3_qpack_encoder_del().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const nghttp3_mem *mem = nghttp3_mem_default();
    assert(mem != NULL);

    nghttp3_qpack_encoder *enc = NULL;
    int rv = nghttp3_qpack_encoder_new(&enc, 4096, mem);
    assert(rv == 0);
    assert(enc != NULL);

    /* Freshly constructed encoder must report zero blocked streams. */
    size_t blocked = nghttp3_qpack_encoder_get_num_blocked_streams(enc);
    assert(blocked == 0);

    nghttp3_qpack_encoder_del(enc);

    nghttp3_qpack_decoder *dec = NULL;
    rv = nghttp3_qpack_decoder_new(&dec, 4096, 16, mem);
    assert(rv == 0);
    assert(dec != NULL);

    nghttp3_qpack_decoder_del(dec);

    /* nghttp3_qpack_*_del documents that passing NULL is a no-op. */
    nghttp3_qpack_encoder_del(NULL);
    nghttp3_qpack_decoder_del(NULL);

    printf("nghttp3 qpack lifecycle test OK\n");
    return 0;
}
