/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openjpeg: stream; calls opj_stream_default_create(), opj_stream_set_read_function(), opj_stream_set_skip_function(), opj_stream_set_seek_function().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openjpeg.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* A minimal in-memory buffer the callbacks can read from. */
typedef struct mem_src {
    const unsigned char *data;
    OPJ_SIZE_T size;
    OPJ_OFF_T pos;
} mem_src_t;

static OPJ_SIZE_T cb_read(void *p_buffer, OPJ_SIZE_T p_nb_bytes,
                          void *p_user_data) {
    mem_src_t *src = (mem_src_t *)p_user_data;
    OPJ_SIZE_T remaining = (OPJ_SIZE_T)(src->size - (OPJ_SIZE_T)src->pos);
    OPJ_SIZE_T n = p_nb_bytes < remaining ? p_nb_bytes : remaining;
    if (n == 0) {
        return (OPJ_SIZE_T)-1; /* documented EOF sentinel */
    }
    memcpy(p_buffer, src->data + src->pos, n);
    src->pos += (OPJ_OFF_T)n;
    return n;
}

static OPJ_OFF_T cb_skip(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
    mem_src_t *src = (mem_src_t *)p_user_data;
    src->pos += p_nb_bytes;
    return p_nb_bytes;
}

static OPJ_BOOL cb_seek(OPJ_OFF_T p_nb_bytes, void *p_user_data) {
    mem_src_t *src = (mem_src_t *)p_user_data;
    src->pos = p_nb_bytes;
    return OPJ_TRUE;
}

static void cb_free(void *p_user_data) {
    /* No-op — the buffer is stack-allocated in main. */
    (void)p_user_data;
}

int main(void) {
    /* Input-oriented stream. */
    opj_stream_t *in = opj_stream_default_create(OPJ_TRUE);
    assert(in != NULL);

    static const unsigned char payload[] = {0x00, 0x01, 0x02, 0x03, 0x04};
    mem_src_t src = {payload, sizeof(payload), 0};

    opj_stream_set_read_function(in, cb_read);
    opj_stream_set_skip_function(in, cb_skip);
    opj_stream_set_seek_function(in, cb_seek);
    opj_stream_set_user_data(in, &src, cb_free);
    opj_stream_set_user_data_length(in, sizeof(payload));

    opj_stream_destroy(in);

    /* Output-oriented stream. */
    opj_stream_t *out = opj_stream_default_create(OPJ_FALSE);
    assert(out != NULL);
    opj_stream_destroy(out);

    /* Custom-sized stream — independent factory entry point. */
    opj_stream_t *sized = opj_stream_create(8192, OPJ_TRUE);
    assert(sized != NULL);
    opj_stream_destroy(sized);

    printf("openjpeg stream test OK\n");
    return 0;
}
