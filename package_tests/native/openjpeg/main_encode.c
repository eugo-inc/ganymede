/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openjpeg: encode; calls opj_image_create(), opj_set_default_encoder_parameters(), opj_create_compress(), opj_setup_encoder().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openjpeg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* In-memory sink — write callback appends to a growable buffer. */
typedef struct mem_sink {
    unsigned char *data;
    OPJ_SIZE_T capacity;
    OPJ_OFF_T pos;
    OPJ_OFF_T written;
} mem_sink_t;

static OPJ_SIZE_T cb_write(void *p_buffer, OPJ_SIZE_T p_nb_bytes,
                           void *p_user_data) {
    mem_sink_t *sink = (mem_sink_t *)p_user_data;
    if ((OPJ_SIZE_T)(sink->pos + (OPJ_OFF_T)p_nb_bytes) > sink->capacity) {
        return (OPJ_SIZE_T)-1;
    }
    memcpy(sink->data + sink->pos, p_buffer, p_nb_bytes);
    sink->pos += (OPJ_OFF_T)p_nb_bytes;
    if (sink->pos > sink->written) {
        sink->written = sink->pos;
    }
    return p_nb_bytes;
}

static OPJ_OFF_T cb_skip(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
    mem_sink_t *sink = (mem_sink_t *)p_user_data;
    sink->pos += p_nb_bytes;
    return p_nb_bytes;
}

static OPJ_BOOL cb_seek(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
    mem_sink_t *sink = (mem_sink_t *)p_user_data;
    if (p_nb_bytes < 0 || (OPJ_SIZE_T)p_nb_bytes > sink->capacity) {
        return OPJ_FALSE;
    }
    sink->pos = p_nb_bytes;
    return OPJ_TRUE;
}

int main(void) {
    opj_image_cmptparm_t cmptparms[1];
    memset(cmptparms, 0, sizeof(cmptparms));
    cmptparms[0].dx = 1;
    cmptparms[0].dy = 1;
    cmptparms[0].w = 32;
    cmptparms[0].h = 32;
    cmptparms[0].x0 = 0;
    cmptparms[0].y0 = 0;
    cmptparms[0].prec = 8;
    cmptparms[0].sgnd = 0;

    opj_image_t *img = opj_image_create(1, cmptparms, OPJ_CLRSPC_GRAY);
    assert(img != NULL);
    img->x0 = 0;
    img->y0 = 0;
    img->x1 = 32;
    img->y1 = 32;
    assert(img->comps[0].data != NULL);

    /* Fill the component with a deterministic gradient so the encoder has
     * non-trivial data to compress. */
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            img->comps[0].data[y * 32 + x] = (x + y) & 0xFF;
        }
    }

    /* Default encoder parameters — lossless 5-3 DWT. */
    opj_cparameters_t cparams;
    opj_set_default_encoder_parameters(&cparams);

    opj_codec_t *enc = opj_create_compress(OPJ_CODEC_J2K);
    assert(enc != NULL);

    OPJ_BOOL ok = opj_setup_encoder(enc, &cparams, img);
    assert(ok == OPJ_TRUE);

    /* Memory-backed output stream. */
    mem_sink_t sink;
    sink.capacity = 64 * 1024;
    sink.data = (unsigned char *)malloc(sink.capacity);
    assert(sink.data != NULL);
    sink.pos = 0;
    sink.written = 0;

    opj_stream_t *stream = opj_stream_default_create(OPJ_FALSE);
    assert(stream != NULL);
    opj_stream_set_write_function(stream, cb_write);
    opj_stream_set_skip_function(stream, cb_skip);
    opj_stream_set_seek_function(stream, cb_seek);
    opj_stream_set_user_data(stream, &sink, NULL);
    opj_stream_set_user_data_length(stream, sink.capacity);

    /* Drive the encode pipeline. */
    ok = opj_start_compress(enc, img, stream);
    assert(ok == OPJ_TRUE);

    ok = opj_encode(enc, stream);
    assert(ok == OPJ_TRUE);

    ok = opj_end_compress(enc, stream);
    assert(ok == OPJ_TRUE);

    opj_stream_destroy(stream);

    assert(sink.written > 4);
    assert(sink.data[0] == 0xFF);
    assert(sink.data[1] == 0x4F);

    free(sink.data);
    opj_destroy_codec(enc);
    opj_image_destroy(img);

    printf("openjpeg encode test OK (%lld bytes)\n", (long long)sink.written);
    return 0;
}
