/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for flac: encode; calls FLAC__stream_encoder_new(), FLAC__stream_encoder_set_channels(), FLAC__stream_encoder_delete(), FLAC__stream_encoder_init_stream().
 */

#include <FLAC/format.h>
#include <FLAC/stream_encoder.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SAMPLE_COUNT 4096u

struct sink {
    unsigned char data[1 << 16];
    size_t size;
};

static FLAC__StreamEncoderWriteStatus write_callback(const FLAC__StreamEncoder *encoder, const FLAC__byte buffer[],
                                                     size_t bytes, uint32_t samples, uint32_t current_frame,
                                                     void *client_data) {
    (void)encoder;
    (void)samples;
    (void)current_frame;
    struct sink *out = client_data;
    if (out->size + bytes > sizeof out->data) {
        return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
    }
    memcpy(out->data + out->size, buffer, bytes);
    out->size += bytes;
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

int main(void) {
    static struct sink out;
    static FLAC__int32 pcm[SAMPLE_COUNT];
    for (uint32_t i = 0; i < SAMPLE_COUNT; ++i) {
        pcm[i] = (FLAC__int32)((i * 37u) % 2000u) - 1000; /* a ramp; no libm needed */
    }

    FLAC__StreamEncoder *encoder = FLAC__stream_encoder_new();
    if (encoder == NULL) {
        fprintf(stderr, "FLAC__stream_encoder_new returned NULL\n");
        return 1;
    }

    const FLAC__bool configured = FLAC__stream_encoder_set_channels(encoder, 1) &&
                                  FLAC__stream_encoder_set_bits_per_sample(encoder, 16) &&
                                  FLAC__stream_encoder_set_sample_rate(encoder, 16000) &&
                                  FLAC__stream_encoder_set_compression_level(encoder, 5) &&
                                  FLAC__stream_encoder_set_total_samples_estimate(encoder, SAMPLE_COUNT);
    if (!configured) {
        fprintf(stderr, "configuring the encoder failed\n");
        FLAC__stream_encoder_delete(encoder);
        return 1;
    }

    const FLAC__StreamEncoderInitStatus init =
        FLAC__stream_encoder_init_stream(encoder, write_callback, NULL, NULL, NULL, &out);
    if (init != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        fprintf(stderr, "init_stream: %s\n", FLAC__StreamEncoderInitStatusString[init]);
        FLAC__stream_encoder_delete(encoder);
        return 1;
    }

    if (!FLAC__stream_encoder_process_interleaved(encoder, pcm, SAMPLE_COUNT)) {
        fprintf(stderr, "process_interleaved: %s\n",
                FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
        FLAC__stream_encoder_delete(encoder);
        return 1;
    }

    if (!FLAC__stream_encoder_finish(encoder)) {
        fprintf(stderr, "finish: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
        FLAC__stream_encoder_delete(encoder);
        return 1;
    }
    FLAC__stream_encoder_delete(encoder);

    if (out.size < 4 || memcmp(out.data, "fLaC", 4) != 0) {
        fprintf(stderr, "encoded stream (%zu bytes) does not start with the fLaC marker\n", out.size);
        return 1;
    }

    printf("libFLAC %s: encoded %u samples into %zu bytes\n", FLAC__VERSION_STRING, SAMPLE_COUNT, out.size);
    return 0;
}
