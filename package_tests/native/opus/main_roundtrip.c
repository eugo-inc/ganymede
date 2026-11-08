/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for opus: roundtrip; calls opus_get_version_string(), opus_encoder_create(), opus_strerror(), opus_decoder_create().
 */

#include <opus/opus.h>

#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define FRAME_SAMPLES 960 /* 20 ms */

int main(void) {
    const char *version = opus_get_version_string();
    if (version == NULL || strncmp(version, "libopus", 7) != 0) {
        fprintf(stderr, "unexpected opus_get_version_string: %s\n", version != NULL ? version : "(null)");
        return 1;
    }

    int error = OPUS_OK;
    OpusEncoder *encoder = opus_encoder_create(SAMPLE_RATE, 1, OPUS_APPLICATION_AUDIO, &error);
    if (encoder == NULL || error != OPUS_OK) {
        fprintf(stderr, "opus_encoder_create: %s\n", opus_strerror(error));
        return 1;
    }
    OpusDecoder *decoder = opus_decoder_create(SAMPLE_RATE, 1, &error);
    if (decoder == NULL || error != OPUS_OK) {
        fprintf(stderr, "opus_decoder_create: %s\n", opus_strerror(error));
        opus_encoder_destroy(encoder);
        return 1;
    }

    opus_int16 pcm[FRAME_SAMPLES];
    opus_int16 decoded[FRAME_SAMPLES];
    unsigned char packet[1500];
    for (int i = 0; i < FRAME_SAMPLES; ++i) {
        pcm[i] = (opus_int16)(((i * 37) % 2000) - 1000); /* a ramp; no libm needed */
    }

    const opus_int32 packet_bytes = opus_encode(encoder, pcm, FRAME_SAMPLES, packet, (opus_int32)sizeof packet);
    const int decoded_samples =
        packet_bytes > 0 ? opus_decode(decoder, packet, packet_bytes, decoded, FRAME_SAMPLES, 0) : -1;
    opus_decoder_destroy(decoder);
    opus_encoder_destroy(encoder);

    if (packet_bytes <= 0 || decoded_samples != FRAME_SAMPLES) {
        fprintf(stderr, "round trip failed: encode=%d decode=%d\n", (int)packet_bytes, decoded_samples);
        return 1;
    }

    printf("%s: %d-sample frame -> %d bytes -> %d samples\n", version, FRAME_SAMPLES, (int)packet_bytes,
           decoded_samples);
    return 0;
}
