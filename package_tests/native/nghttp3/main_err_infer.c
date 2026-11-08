/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp3: err infer; calls nghttp3_err_infer_quic_app_error_code().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* H3 frame errors map to their H3 app counterparts. */
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_FRAME_UNEXPECTED)
           == NGHTTP3_H3_FRAME_UNEXPECTED);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_FRAME_ERROR)
           == NGHTTP3_H3_FRAME_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_MISSING_SETTINGS)
           == NGHTTP3_H3_MISSING_SETTINGS);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_CLOSED_CRITICAL_STREAM)
           == NGHTTP3_H3_CLOSED_CRITICAL_STREAM);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_ID_ERROR)
           == NGHTTP3_H3_ID_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_SETTINGS_ERROR)
           == NGHTTP3_H3_SETTINGS_ERROR);

    /* QPACK errors map to the QPACK app codes. */
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_QPACK_DECOMPRESSION_FAILED)
           == NGHTTP3_QPACK_DECOMPRESSION_FAILED);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_QPACK_ENCODER_STREAM_ERROR)
           == NGHTTP3_QPACK_ENCODER_STREAM_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_QPACK_DECODER_STREAM_ERROR)
           == NGHTTP3_QPACK_DECODER_STREAM_ERROR);

    assert(nghttp3_err_infer_quic_app_error_code(0) == NGHTTP3_H3_NO_ERROR);

    /* Internal-class library errors collapse to H3_INTERNAL_ERROR. */
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_NOMEM)
           == NGHTTP3_H3_INTERNAL_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_H3_INTERNAL_ERROR)
           == NGHTTP3_H3_INTERNAL_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_CALLBACK_FAILURE)
           == NGHTTP3_H3_INTERNAL_ERROR);

    /* Malformed-header / messaging errors collapse to H3_MESSAGE_ERROR. */
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_MALFORMED_HTTP_HEADER)
           == NGHTTP3_H3_MESSAGE_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_MALFORMED_HTTP_MESSAGING)
           == NGHTTP3_H3_MESSAGE_ERROR);

    /* Any error not in the table (e.g. INVALID_ARGUMENT, WOULDBLOCK)
     * falls through to the default branch -> H3_GENERAL_PROTOCOL_ERROR. */
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_INVALID_ARGUMENT)
           == NGHTTP3_H3_GENERAL_PROTOCOL_ERROR);
    assert(nghttp3_err_infer_quic_app_error_code(NGHTTP3_ERR_WOULDBLOCK)
           == NGHTTP3_H3_GENERAL_PROTOCOL_ERROR);

    printf("nghttp3 err_infer test OK\n");
    return 0;
}
