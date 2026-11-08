/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ffmpeg: gpu decode; calls av_log_default_callback(), avcodec_receive_frame(), av_err2str(), av_get_pix_fmt_name().
 */

#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/log.h>
#include <libavutil/pixdesc.h>

#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sample_h264.h"

#define MAX_FRAMES 16

/* EVERY message FFmpeg logged at error level since the current step began (forget_errors() starts a step), joined
 * with "; ". All of them, not just the last: the message naming the cause is often not the last one. A missing
 * libnvcuvid logs "Cannot load libnvcuvid.so.1" (compat/cuda/dynlink_loader.h) and then "Failed loading nvcuvid."
 * (libavcodec/nvdec.c, libavcodec/cuviddec.c), and on the nvdec path get_format() failures follow; FFmpeg's
 * CUDA check logs "<call> failed", " -> <CUDA error name>: <text>" and "\n" as three calls (libavutil/cuda_check.h).
 * Every message still reaches stderr through FFmpeg's default logger. A mutex, because decoder threads log too. */
static char logged_errors[2048];
static size_t logged_errors_length;
static pthread_mutex_t logged_errors_lock = PTHREAD_MUTEX_INITIALIZER;

static void capture_error_messages(void *context, int level, const char *format, va_list arguments) {
    va_list copy;
    va_copy(copy, arguments);
    av_log_default_callback(context, level, format, arguments);
    if (level <= AV_LOG_ERROR) {
        pthread_mutex_lock(&logged_errors_lock);
        const size_t room = sizeof(logged_errors) - logged_errors_length;
        if (room > 1) {
            const int written = vsnprintf(logged_errors + logged_errors_length, room, format, copy);
            if (written > 0) {
                const size_t end = logged_errors_length + ((size_t)written < room - 1 ? (size_t)written : room - 1);
                for (size_t i = logged_errors_length; i < end; ++i) {
                    if (logged_errors[i] == '\n') {
                        logged_errors[i] = ';'; /* one line on the FAIL message */
                    }
                }
                logged_errors_length = end;
            }
        }
        pthread_mutex_unlock(&logged_errors_lock);
    }
    va_end(copy);
}

static void forget_errors(void) {
    pthread_mutex_lock(&logged_errors_lock);
    logged_errors_length = 0;
    logged_errors[0] = '\0';
    pthread_mutex_unlock(&logged_errors_lock);
}

/* The joined messages without the trailing separator, or a note that there were none. */
static const char *errors_logged(void) {
    pthread_mutex_lock(&logged_errors_lock);
    while (logged_errors_length > 0 &&
           (logged_errors[logged_errors_length - 1] == ';' || logged_errors[logged_errors_length - 1] == ' ')) {
        logged_errors[--logged_errors_length] = '\0';
    }
    const int is_empty = logged_errors_length == 0;
    pthread_mutex_unlock(&logged_errors_lock);
    return is_empty ? "(FFmpeg logged no error)" : logged_errors;
}

static int errors_mention(const char *text) {
    return strstr(errors_logged(), text) != NULL;
}

static const char *nvcuvid_hint(void) {
    return errors_mention("nvcuvid")
               ? " -- libnvcuvid.so.1 (the NVIDIA driver's video decode library) did not load"
               : "";
}

typedef struct {
    int frame_count;
    uint64_t luma_hashes[MAX_FRAMES];
    uint64_t chroma_hashes[MAX_FRAMES];
} DecodedClip;

static uint64_t fnv1a_update(uint64_t hash, const uint8_t *bytes, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 0x100000001b3ULL;
    }
    return hash;
}

/* Hashes an 8-bit 4:2:0 frame as Y rows, then chroma as interleaved U,V pairs, so a planar yuv420p frame (the
 * software decoder) and a semi-planar nv12 frame (what the GPU returns) hash the same when their pixels agree. */
static int hash_frame(const AVFrame *frame, uint64_t *luma_hash, uint64_t *chroma_hash) {
    const int chroma_width = (frame->width + 1) >> 1;
    const int chroma_height = (frame->height + 1) >> 1;
    uint64_t luma = 0xcbf29ce484222325ULL;
    uint64_t chroma = 0xcbf29ce484222325ULL;
    for (int y = 0; y < frame->height; ++y) {
        luma = fnv1a_update(luma, frame->data[0] + (ptrdiff_t)y * frame->linesize[0], (size_t)frame->width);
    }
    if (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUVJ420P) {
        for (int y = 0; y < chroma_height; ++y) {
            const uint8_t *u = frame->data[1] + (ptrdiff_t)y * frame->linesize[1];
            const uint8_t *v = frame->data[2] + (ptrdiff_t)y * frame->linesize[2];
            for (int x = 0; x < chroma_width; ++x) {
                const uint8_t pair[2] = {u[x], v[x]};
                chroma = fnv1a_update(chroma, pair, 2);
            }
        }
    } else if (frame->format == AV_PIX_FMT_NV12) {
        for (int y = 0; y < chroma_height; ++y) {
            chroma = fnv1a_update(chroma, frame->data[1] + (ptrdiff_t)y * frame->linesize[1], (size_t)chroma_width * 2);
        }
    } else {
        return -1;
    }
    *luma_hash = luma;
    *chroma_hash = chroma;
    return 0;
}

static enum AVPixelFormat pick_cuda_format(AVCodecContext *context, const enum AVPixelFormat *formats) {
    (void)context;
    for (const enum AVPixelFormat *format = formats; *format != AV_PIX_FMT_NONE; ++format) {
        if (*format == AV_PIX_FMT_CUDA) {
            return AV_PIX_FMT_CUDA;
        }
    }
    /* No CUDA format offered: this FFmpeg has no nvdec hwaccel, or it failed to start (FFmpeg drops a format whose
     * hwaccel will not initialise). Refusing every other format turns a silent CPU fall-back into a decode error. */
    fprintf(stderr, "  the decoder offered no CUDA output format -- no nvdec hwaccel in this build, or it did not "
                    "start\n");
    return AV_PIX_FMT_NONE;
}

static int receive_frames(AVCodecContext *context, AVFrame *frame, AVFrame *download, int is_gpu, const char *label,
                          DecodedClip *clip) {
    for (;;) {
        const int result = avcodec_receive_frame(context, frame);
        if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            return 0;
        }
        if (result < 0) {
            fprintf(stderr, "FAIL [%s]: avcodec_receive_frame: %s; FFmpeg's errors: %s\n", label, av_err2str(result),
                    errors_logged());
            return 1;
        }
        const AVFrame *pixels = frame;
        if (is_gpu) {
            if (frame->format != AV_PIX_FMT_CUDA || frame->hw_frames_ctx == NULL) {
                fprintf(stderr, "FAIL [%s]: frame %d came back as %s, not a GPU (cuda) frame\n", label,
                        clip->frame_count, av_get_pix_fmt_name((enum AVPixelFormat)frame->format));
                av_frame_unref(frame);
                return 1;
            }
            av_frame_unref(download);
            const int copied = av_hwframe_transfer_data(download, frame, 0);
            if (copied < 0) {
                fprintf(stderr, "FAIL [%s]: copying frame %d off the GPU: %s; FFmpeg's errors: %s\n", label,
                        clip->frame_count, av_err2str(copied), errors_logged());
                av_frame_unref(frame);
                return 1;
            }
            download->width = frame->width;
            download->height = frame->height;
            pixels = download;
        }
        if (clip->frame_count >= MAX_FRAMES) {
            fprintf(stderr, "FAIL [%s]: more than %d frames\n", label, MAX_FRAMES);
            av_frame_unref(frame);
            return 1;
        }
        if (hash_frame(pixels, &clip->luma_hashes[clip->frame_count], &clip->chroma_hashes[clip->frame_count]) != 0) {
            fprintf(stderr, "FAIL [%s]: frame %d has pixel format %s, expected yuv420p or nv12\n", label,
                    clip->frame_count, av_get_pix_fmt_name((enum AVPixelFormat)pixels->format));
            av_frame_unref(frame);
            return 1;
        }
        clip->frame_count += 1;
        av_frame_unref(frame);
    }
}

/* Decodes the whole clip with `decoder_name`; on the GPU when `device` is non-NULL. Returns failed checks. */
static int decode_clip(const char *decoder_name, AVBufferRef *device, const char *label, DecodedClip *clip) {
    memset(clip, 0, sizeof(*clip));
    const AVCodec *decoder = avcodec_find_decoder_by_name(decoder_name);
    if (decoder == NULL) {
        fprintf(stderr, "FAIL [%s]: decoder `%s` is not registered\n", label, decoder_name);
        return 1;
    }
    AVCodecContext *context = avcodec_alloc_context3(decoder);
    AVCodecParserContext *parser = av_parser_init(AV_CODEC_ID_H264);
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    AVFrame *download = av_frame_alloc();
    int failures = 0;
    if (context == NULL || parser == NULL || packet == NULL || frame == NULL || download == NULL) {
        fprintf(stderr, "FAIL [%s]: out of memory setting up the decoder\n", label);
        failures = 1;
        goto done;
    }
    context->pkt_timebase = (AVRational){1, 25};
    if (device != NULL) {
        context->hw_device_ctx = av_buffer_ref(device);
        context->get_format = pick_cuda_format;
    }
    forget_errors();
    int result = avcodec_open2(context, decoder, NULL);
    if (result < 0) {
        fprintf(stderr, "FAIL [%s]: avcodec_open2(%s): %s; FFmpeg's errors: %s%s\n", label, decoder_name,
                av_err2str(result), errors_logged(), device != NULL ? nvcuvid_hint() : "");
        failures = 1;
        goto done;
    }

    const uint8_t *cursor = demo_h264_annexb;
    size_t remaining = sizeof(demo_h264_annexb);
    for (;;) {
        uint8_t *packet_data = NULL;
        int packet_size = 0;
        const int consumed = av_parser_parse2(parser, context, &packet_data, &packet_size, cursor, (int)remaining,
                                              AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (consumed < 0) {
            fprintf(stderr, "FAIL [%s]: the H.264 parser rejected the clip: %s\n", label, av_err2str(consumed));
            failures = 1;
            goto done;
        }
        cursor += consumed;
        remaining -= (size_t)consumed;
        const int is_flush = remaining == 0 && consumed == 0;
        if (packet_size > 0) {
            if (av_new_packet(packet, packet_size) < 0) {
                fprintf(stderr, "FAIL [%s]: out of memory for a packet\n", label);
                failures = 1;
                goto done;
            }
            memcpy(packet->data, packet_data, (size_t)packet_size);
            forget_errors();
            result = avcodec_send_packet(context, packet);
            av_packet_unref(packet);
            if (result < 0) {
                const char *hint = nvcuvid_hint();
                fprintf(stderr, "FAIL [%s]: avcodec_send_packet: %s; FFmpeg's errors: %s%s\n", label,
                        av_err2str(result), errors_logged(),
                        device == NULL ? ""
                        : hint[0] != '\0'
                            ? hint
                            : " -- the GPU decoder did not start; FFmpeg's errors on this line say why");
                failures = 1;
                goto done;
            }
            failures += receive_frames(context, frame, download, device != NULL, label, clip);
            if (failures != 0) {
                goto done;
            }
        }
        if (is_flush) {
            break;
        }
    }
    avcodec_send_packet(context, NULL);
    failures += receive_frames(context, frame, download, device != NULL, label, clip);

done:
    av_frame_free(&download);
    av_frame_free(&frame);
    av_packet_free(&packet);
    av_parser_close(parser);
    avcodec_free_context(&context);
    return failures;
}

static int compare_clips(const DecodedClip *reference, const DecodedClip *clip, const char *label) {
    if (clip->frame_count != DEMO_H264_FRAME_COUNT) {
        fprintf(stderr, "FAIL [%s]: decoded %d frames, the clip has %d\n", label, clip->frame_count,
                DEMO_H264_FRAME_COUNT);
        return 1;
    }
    int failures = 0;
    for (int i = 0; i < clip->frame_count; ++i) {
        if (clip->luma_hashes[i] != reference->luma_hashes[i] || clip->chroma_hashes[i] != reference->chroma_hashes[i]) {
            fprintf(stderr,
                    "FAIL [%s]: frame %d differs from the CPU decode (Y 0x%016" PRIx64 " vs 0x%016" PRIx64
                    ", UV 0x%016" PRIx64 " vs 0x%016" PRIx64 ")\n",
                    label, i, clip->luma_hashes[i], reference->luma_hashes[i], clip->chroma_hashes[i],
                    reference->chroma_hashes[i]);
            failures += 1;
        }
    }
    if (failures == 0) {
        printf("%s: %d frames decoded on the GPU, pixel-identical to the CPU decode\n", label, clip->frame_count);
    }
    return failures;
}

int main(void) {
    int failures = 0;
    av_log_set_callback(capture_error_messages);

    DecodedClip reference;
    failures += decode_clip("h264", NULL, "cpu h264", &reference);
    if (failures == 0 && reference.frame_count != DEMO_H264_FRAME_COUNT) {
        fprintf(stderr, "FAIL [cpu h264]: decoded %d frames, the clip has %d\n", reference.frame_count,
                DEMO_H264_FRAME_COUNT);
        failures += 1;
    }
    if (failures != 0) {
        fprintf(stderr, "ffmpeg GPU decode: the CPU reference decode failed, so nothing on the GPU was checked\n");
        return 2;
    }
    printf("cpu h264: %d frames of %dx%d (the reference)\n", reference.frame_count, DEMO_H264_WIDTH, DEMO_H264_HEIGHT);

    AVBufferRef *device = NULL;
    forget_errors();
    const int created = av_hwdevice_ctx_create(&device, AV_HWDEVICE_TYPE_CUDA, NULL, NULL, 0);
    if (created < 0) {
        const char *cause = errors_mention("CUDA_ERROR_")
                                ? "the CUDA driver loaded and refused the request with the CUDA error quoted on this line"
                            : errors_mention("Could not dynamically load CUDA")
                                ? "libcuda.so.1 did not load (no NVIDIA driver found)"
                                : "FFmpeg logged no CUDA error; its messages on stderr say what failed";
        fprintf(stderr,
                "FAIL: could not create a CUDA device (%s) although --with gpu was given; FFmpeg's errors: %s -- %s\n",
                av_err2str(created), errors_logged(), cause);
        return 3;
    }

    DecodedClip clip;
    const int nvdec_failures = decode_clip("h264", device, "nvdec h264", &clip);
    failures += nvdec_failures != 0 ? nvdec_failures : compare_clips(&reference, &clip, "nvdec h264");
    const int cuvid_failures = decode_clip("h264_cuvid", device, "cuvid h264_cuvid", &clip);
    failures += cuvid_failures != 0 ? cuvid_failures : compare_clips(&reference, &clip, "cuvid h264_cuvid");
    av_buffer_unref(&device);

    if (failures != 0) {
        fprintf(stderr, "ffmpeg GPU decode: %d check(s) failed\n", failures);
        return 4;
    }
    printf("ffmpeg GPU decode OK: nvdec and cuvid decode H.264 on the GPU, bit-exact against the CPU\n");
    return 0;
}
