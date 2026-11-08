/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ffmpeg: av1 software; calls avcodec_receive_frame(), av_err2str(), av_get_pix_fmt_name(), av_frame_unref().
 */

#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sample_av1.h"

static uint64_t fnv1a_update(uint64_t hash, const uint8_t *bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

static uint64_t hash_frame(const AVFrame *frame, int bytes_per_sample) {
    const int chroma_width = (frame->width + 1) >> 1;
    const int chroma_height = (frame->height + 1) >> 1;
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (int plane = 0; plane < 3; ++plane) {
        const int width = plane == 0 ? frame->width : chroma_width;
        const int height = plane == 0 ? frame->height : chroma_height;
        for (int y = 0; y < height; ++y) {
            hash = fnv1a_update(hash, frame->data[plane] + (ptrdiff_t)y * frame->linesize[plane],
                                (size_t)width * (size_t)bytes_per_sample);
        }
    }
    return hash;
}

static int receive_frames(AVCodecContext *context, AVFrame *frame, const struct demo_av1_sample *sample,
                          int *frame_count) {
    const enum AVPixelFormat expected_format =
        sample->bits_per_sample > 8 ? AV_PIX_FMT_YUV420P10LE : AV_PIX_FMT_YUV420P;
    int failures = 0;
    for (;;) {
        const int result = avcodec_receive_frame(context, frame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return failures;
        }
        if (result < 0) {
            fprintf(stderr, "FAIL [%s]: avcodec_receive_frame: %s\n", sample->name, av_err2str(result));
            return failures + 1;
        }
        if (*frame_count >= sample->frame_count) {
            fprintf(stderr, "FAIL [%s]: more than the %d expected frames\n", sample->name, sample->frame_count);
            failures += 1;
        } else if (frame->width != sample->width || frame->height != sample->height ||
                   frame->format != expected_format) {
            fprintf(stderr, "FAIL [%s]: frame %d is %dx%d %s, expected %dx%d %s\n", sample->name, *frame_count,
                    frame->width, frame->height, av_get_pix_fmt_name((enum AVPixelFormat)frame->format),
                    sample->width, sample->height, av_get_pix_fmt_name(expected_format));
            failures += 1;
        } else {
            const uint64_t hash = hash_frame(frame, sample->bits_per_sample > 8 ? 2 : 1);
            if (hash != sample->frame_hashes[*frame_count]) {
                fprintf(stderr,
                        "FAIL [%s]: frame %d hashes to 0x%016" PRIx64 ", expected 0x%016" PRIx64 "\n",
                        sample->name, *frame_count, hash, sample->frame_hashes[*frame_count]);
                failures += 1;
            }
        }
        *frame_count += 1;
        av_frame_unref(frame);
    }
}

static uint32_t read_le32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

static int decode_sample(const AVCodec *decoder, const struct demo_av1_sample *sample) {
    AVCodecContext *context = avcodec_alloc_context3(decoder);
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    int failures = 0;
    int frame_count = 0;
    if (context == NULL || packet == NULL || frame == NULL) {
        fprintf(stderr, "FAIL [%s]: out of memory setting up the decoder\n", sample->name);
        failures = 1;
        goto done;
    }
    int result = avcodec_open2(context, decoder, NULL);
    if (result < 0) {
        fprintf(stderr, "FAIL [%s]: avcodec_open2(%s): %s\n", sample->name, decoder->name, av_err2str(result));
        failures = 1;
        goto done;
    }
    size_t offset = (size_t)(sample->ivf[6] | (sample->ivf[7] << 8));
    while (offset + 12 <= sample->ivf_size) {
        const uint32_t frame_size = read_le32(sample->ivf + offset);
        offset += 12;
        if (frame_size == 0 || offset + frame_size > sample->ivf_size || av_new_packet(packet, (int)frame_size) < 0) {
            fprintf(stderr, "FAIL [%s]: bad IVF frame at byte %zu\n", sample->name, offset);
            failures = 1;
            goto done;
        }
        memcpy(packet->data, sample->ivf + offset, frame_size);
        offset += frame_size;
        result = avcodec_send_packet(context, packet);
        av_packet_unref(packet);
        if (result < 0) {
            fprintf(stderr, "FAIL [%s]: avcodec_send_packet: %s\n", sample->name, av_err2str(result));
            failures = 1;
            goto done;
        }
        failures += receive_frames(context, frame, sample, &frame_count);
    }
    avcodec_send_packet(context, NULL);
    failures += receive_frames(context, frame, sample, &frame_count);
    if (failures == 0 && frame_count != sample->frame_count) {
        fprintf(stderr, "FAIL [%s]: decoded %d frames, expected %d\n", sample->name, frame_count, sample->frame_count);
        failures += 1;
    }
    if (failures == 0) {
        printf("%s: %d frames decoded by `%s` on the CPU; every frame matches the expected hashes\n", sample->name,
               frame_count, decoder->name);
    }

done:
    av_frame_free(&frame);
    av_packet_free(&packet);
    avcodec_free_context(&context);
    return failures;
}

int main(void) {
    int failures = 0;

    const AVCodec *libdav1d = avcodec_find_decoder_by_name("libdav1d");
    if (libdav1d == NULL) {
        fprintf(stderr, "FAIL: decoder `libdav1d` is not available in this libavcodec\n");
        failures += 1;
    }
    const AVCodec *default_av1 = avcodec_find_decoder(AV_CODEC_ID_AV1);
    if (default_av1 == NULL || strcmp(default_av1->name, "libdav1d") != 0) {
        fprintf(stderr, "FAIL: the default AV1 decoder is `%s`, not `libdav1d`\n",
                default_av1 != NULL ? default_av1->name : "(none)");
        failures += 1;
    } else {
        printf("default AV1 decoder: libdav1d\n");
    }

    if (libdav1d != NULL) {
        for (size_t i = 0; i < sizeof(demo_av1_samples) / sizeof(demo_av1_samples[0]); ++i) {
            failures += decode_sample(libdav1d, &demo_av1_samples[i]);
        }
    }

    if (failures != 0) {
        fprintf(stderr, "ffmpeg AV1 software decode: %d check(s) failed\n", failures);
        return 2;
    }
    printf("ffmpeg AV1 software decode OK: libdav1d decodes 8-bit and 10-bit AV1 bit-exactly\n");
    return 0;
}
