/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opus: roundtrip; calls opus_get_version_string(), opus_encoder_create(), opus_strerror(), opus_decoder_create().
 */

#include <opus/opus.h>

#include <stdio.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define FRAME_SAMPLES 960 /* 20 ms */
#define FRAMES 25         /* 500 ms */
#define WARMUP_FRAMES 5
#define TOTAL_SAMPLES (FRAME_SAMPLES * FRAMES)
#define AMPLITUDE 8000.0

static opus_int16 pcm[TOTAL_SAMPLES];
static opus_int16 decoded[TOTAL_SAMPLES];

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
    if (opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000)) != OPUS_OK) {
        fprintf(stderr, "opus_encoder_ctl(OPUS_SET_BITRATE) failed\n");
        return 1;
    }
    opus_int32 lookahead = -1;
    if (opus_encoder_ctl(encoder, OPUS_GET_LOOKAHEAD(&lookahead)) != OPUS_OK || lookahead < 0 ||
        lookahead >= FRAME_SAMPLES) {
        fprintf(stderr, "opus_encoder_ctl(OPUS_GET_LOOKAHEAD) failed or out of range: %d\n", (int)lookahead);
        return 1;
    }

    /* 440 Hz sine: y[n] = 2cos(w) y[n-1] - y[n-2], y[0] = 0, y[1] = sin(w). */
    const double two_cos_w = 1.9966836332280566;
    double y_prev = 0.0;
    double y = 0.057564026959567284;
    pcm[0] = 0;
    for (int i = 1; i < TOTAL_SAMPLES; ++i) {
        pcm[i] = (opus_int16)(AMPLITUDE * y);
        const double y_next = two_cos_w * y - y_prev;
        y_prev = y;
        y = y_next;
    }

    unsigned char packet[1500];
    for (int f = 0; f < FRAMES; ++f) {
        const opus_int32 packet_bytes =
            opus_encode(encoder, pcm + f * FRAME_SAMPLES, FRAME_SAMPLES, packet, (opus_int32)sizeof packet);
        if (packet_bytes <= 0) {
            fprintf(stderr, "opus_encode failed on frame %d: %d\n", f, (int)packet_bytes);
            return 1;
        }
        const int got = opus_decode(decoder, packet, packet_bytes, decoded + f * FRAME_SAMPLES, FRAME_SAMPLES, 0);
        if (got != FRAME_SAMPLES) {
            fprintf(stderr, "opus_decode returned %d samples on frame %d, want %d\n", got, f, FRAME_SAMPLES);
            return 1;
        }
    }
    opus_decoder_destroy(decoder);
    opus_encoder_destroy(encoder);

    /* Compare input[n] with decoded[n + lookahead] over the frames after warm-up. */
    double sxx = 0.0, syy = 0.0, sxy = 0.0, see = 0.0;
    for (int n = WARMUP_FRAMES * FRAME_SAMPLES; n + lookahead < TOTAL_SAMPLES; ++n) {
        const double x = pcm[n];
        const double d = decoded[n + lookahead];
        sxx += x * x;
        syy += d * d;
        sxy += x * d;
        see += (x - d) * (x - d);
    }
    if (syy <= 0.0) {
        fprintf(stderr, "decoded audio is silent (energy 0): the codec returned no signal\n");
        return 1;
    }
    const double r2 = sxy > 0.0 ? (sxy * sxy) / (sxx * syy) : 0.0;
    if (r2 < 0.90) {
        fprintf(stderr, "decoded audio does not match the input: r^2 = %.4f (want >= 0.90, lag %d)\n", r2,
                (int)lookahead);
        return 1;
    }
    if (see * 10.0 > sxx) {
        fprintf(stderr, "decoded audio is too noisy: signal/error energy = %.2f (want >= 10, i.e. 10 dB)\n",
                see > 0.0 ? sxx / see : 0.0);
        return 1;
    }

    printf("%s: %d frames round-tripped, lookahead %d, r^2 %.4f, signal/error energy %.1f\n", version, FRAMES,
           (int)lookahead, r2, see > 0.0 ? sxx / see : 0.0);
    return 0;
}
