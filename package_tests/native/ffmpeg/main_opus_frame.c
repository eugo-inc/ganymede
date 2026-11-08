/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ffmpeg: opus frame; calls av_strerror(), avcodec_find_encoder_by_name(), avcodec_alloc_context3(), av_channel_layout_copy().
 */

#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>

#include <stdio.h>

static int fail(const char *what, int err) {
    char message[AV_ERROR_MAX_STRING_SIZE] = {0};
    av_strerror(err, message, sizeof(message));
    fprintf(stderr, "FAIL: %s: %s (%d)\n", what, message, err);
    return 2;
}

int main(void) {
    const AVCodec *codec = avcodec_find_encoder_by_name("libopus");
    if (codec == NULL) {
        fprintf(stderr, "FAIL: encoder `libopus` is not registered\n");
        return 2;
    }

    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    if (ctx == NULL) {
        fprintf(stderr, "FAIL: avcodec_alloc_context3\n");
        return 2;
    }
    const AVChannelLayout mono = AV_CHANNEL_LAYOUT_MONO;
    ctx->sample_rate = 48000;
    ctx->sample_fmt = AV_SAMPLE_FMT_FLT; /* one of the two formats the wrapper accepts: S16, FLT */
    ctx->bit_rate = 64000;
    ctx->time_base = (AVRational){1, 48000};
    int err = av_channel_layout_copy(&ctx->ch_layout, &mono);
    if (err < 0) {
        return fail("av_channel_layout_copy", err);
    }

    err = avcodec_open2(ctx, codec, NULL);
    if (err < 0) {
        return fail("avcodec_open2(libopus)", err);
    }
    if (ctx->frame_size != 960) {
        fprintf(stderr, "FAIL: libopus frame_size = %d, expected 960 (20 ms at 48 kHz)\n", ctx->frame_size);
        return 3;
    }

    AVFrame *frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    if (frame == NULL || packet == NULL) {
        fprintf(stderr, "FAIL: av_frame_alloc / av_packet_alloc\n");
        return 2;
    }
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->sample_rate = ctx->sample_rate;
    frame->pts = 0;
    err = av_channel_layout_copy(&frame->ch_layout, &ctx->ch_layout);
    if (err < 0) {
        return fail("av_channel_layout_copy(frame)", err);
    }
    err = av_frame_get_buffer(frame, 0);
    if (err < 0) {
        return fail("av_frame_get_buffer", err);
    }
    float *samples = (float *)frame->data[0];
    for (int i = 0; i < frame->nb_samples; ++i) {
        samples[i] = 0.25f * ((float)(i % 109) / 54.5f - 1.0f);
    }

    err = avcodec_send_frame(ctx, frame);
    if (err < 0) {
        return fail("avcodec_send_frame", err);
    }
    err = avcodec_send_frame(ctx, NULL); /* flush */
    if (err < 0) {
        return fail("avcodec_send_frame(flush)", err);
    }

    int packets = 0;
    int bytes = 0;
    for (;;) {
        err = avcodec_receive_packet(ctx, packet);
        if (err == AVERROR_EOF) {
            break;
        }
        if (err < 0) {
            return fail("avcodec_receive_packet", err);
        }
        packets += 1;
        bytes += packet->size;
        av_packet_unref(packet);
    }
    if (packets < 1 || bytes <= 0) {
        fprintf(stderr, "FAIL: libopus produced %d packet(s), %d byte(s) for one 20 ms frame\n", packets, bytes);
        return 4;
    }

    printf("ffmpeg libopus OK: %d packet(s), %d byte(s) from one 20 ms frame\n", packets, bytes);
    av_packet_free(&packet);
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
    return 0;
}
