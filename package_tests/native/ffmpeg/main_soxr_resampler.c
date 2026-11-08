/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ffmpeg: soxr resampler; calls av_strerror(), swr_alloc_set_opts2(), av_opt_set_int(), swr_init().
 */

#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#include <stdint.h>
#include <stdio.h>

static int fail(const char *what, int err) {
    char message[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(err, message, sizeof(message));
    fprintf(stderr, "FAIL: %s: %s (%d)\n", what, message, err);
    return 2;
}

int main(void) {
    const AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
    const AVChannelLayout mono = AV_CHANNEL_LAYOUT_MONO;
    SwrContext *swr = NULL;
    int err = swr_alloc_set_opts2(&swr, &mono, AV_SAMPLE_FMT_FLT, 16000, &stereo, AV_SAMPLE_FMT_S16, 44100, 0, NULL);
    if (err < 0) {
        return fail("swr_alloc_set_opts2", err);
    }
    err = av_opt_set_int(swr, "resampler", SWR_ENGINE_SOXR, 0);
    if (err < 0) {
        return fail("av_opt_set_int(resampler=soxr)", err);
    }
    err = swr_init(swr);
    if (err < 0) {
        return fail("swr_init with resampler=soxr (this libswresample has no soxr engine?)", err);
    }

    enum { IN_SAMPLES = 441, OUT_CAPACITY = 1024 };
    static int16_t input[IN_SAMPLES * 2];
    for (int i = 0; i < IN_SAMPLES; ++i) {
        const int16_t v = (int16_t)(((i % 100) - 50) * 200); /* a 441 Hz sawtooth, both channels */
        input[2 * i] = v;
        input[2 * i + 1] = v;
    }
    static float output[OUT_CAPACITY];
    const uint8_t *in_planes[1] = {(const uint8_t *)input};
    uint8_t *out_planes[1] = {(uint8_t *)output};

    int produced = swr_convert(swr, out_planes, OUT_CAPACITY, in_planes, IN_SAMPLES);
    if (produced < 0) {
        return fail("swr_convert", produced);
    }
    int total = produced;
    for (;;) { /* flush the resampler's delay line */
        out_planes[0] = (uint8_t *)(output + total);
        produced = swr_convert(swr, out_planes, OUT_CAPACITY - total, NULL, 0);
        if (produced < 0) {
            return fail("swr_convert(flush)", produced);
        }
        if (produced == 0 || total + produced >= OUT_CAPACITY) {
            total += produced;
            break;
        }
        total += produced;
    }
    swr_free(&swr);

    /* 441 input samples at 44.1 kHz are 10 ms, i.e. 160 samples at 16 kHz once the delay line is flushed. */
    if (total <= 0 || total > 320) {
        fprintf(stderr, "FAIL: soxr produced %d output samples for 10 ms of input, expected about 160\n", total);
        return 3;
    }
    printf("ffmpeg soxr resampler OK: 441 samples @44.1 kHz stereo -> %d samples @16 kHz mono\n", total);
    return 0;
}
