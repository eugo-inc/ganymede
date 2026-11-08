/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libvpx: encode decode vpx; calls vpx_codec_enc_config_default(), vpx_codec_enc_init(), vpx_codec_error(), vpx_codec_dec_init().
 */

#include <vpx/vp8cx.h>
#include <vpx/vp8dx.h>
#include <vpx/vpx_decoder.h>
#include <vpx/vpx_encoder.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { WIDTH = 64, HEIGHT = 64, FRAMES = 10 };

static int failures = 0;

#define CHECK(cond, ...)                                    \
    do {                                                    \
        if (!(cond)) {                                      \
            fprintf(stderr, "FAIL: " __VA_ARGS__);          \
            fputc('\n', stderr);                            \
            ++failures;                                     \
        }                                                   \
    } while (0)

static int sample(int frame, int x, int y, int bit_depth) {
    const int max = (1 << bit_depth) - 1;
    return ((x * 3 + y * 2 + frame * 5) * max / (WIDTH * 3 + HEIGHT * 2 + FRAMES * 5)) & max;
}

static void fill(vpx_image_t *img, int frame, int bit_depth) {
    for (int plane = 0; plane < 3; ++plane) {
        const int w = plane ? (WIDTH + 1) / 2 : WIDTH;
        const int h = plane ? (HEIGHT + 1) / 2 : HEIGHT;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                const int v = plane ? (1 << (bit_depth - 1)) : sample(frame, x, y, bit_depth);
                if (bit_depth > 8) {
                    ((uint16_t *)(img->planes[plane] + y * img->stride[plane]))[x] = (uint16_t)v;
                } else {
                    img->planes[plane][y * img->stride[plane] + x] = (uint8_t)v;
                }
            }
        }
    }
}

static double luma_psnr(const vpx_image_t *img, int frame, int bit_depth) {
    const int high = (img->fmt & VPX_IMG_FMT_HIGHBITDEPTH) != 0;
    const double max = (double)((1 << bit_depth) - 1);
    double se = 0.0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            const int got = high ? ((const uint16_t *)(img->planes[0] + y * img->stride[0]))[x]
                                 : img->planes[0][y * img->stride[0] + x];
            const double d = (double)(got - sample(frame, x, y, bit_depth));
            se += d * d;
        }
    }
    const double mse = se / (WIDTH * HEIGHT);
    return mse == 0.0 ? 99.0 : 10.0 * log10(max * max / mse);
}

static void run(const char *label, vpx_codec_iface_t *encoder, vpx_codec_iface_t *decoder, int bit_depth) {
    /* High bit depth is an optional part of libvpx; skip the case when this encoder does not offer it. */
    if (bit_depth > 8 && (vpx_codec_get_caps(encoder) & VPX_CODEC_CAP_HIGHBITDEPTH) == 0) {
        printf("%s: not built in, skipped\n", label);
        return;
    }
    vpx_codec_enc_cfg_t cfg;
    CHECK(vpx_codec_enc_config_default(encoder, &cfg, 0) == VPX_CODEC_OK, "%s: no default encoder settings", label);
    cfg.g_w = WIDTH;
    cfg.g_h = HEIGHT;
    cfg.g_timebase.num = 1;
    cfg.g_timebase.den = 25;
    cfg.rc_target_bitrate = 2000;
    cfg.g_lag_in_frames = 0;
    vpx_codec_flags_t flags = 0;
    vpx_img_fmt_t fmt = VPX_IMG_FMT_I420;
    if (bit_depth > 8) {
        cfg.g_profile = 2;
        cfg.g_bit_depth = VPX_BITS_10;
        cfg.g_input_bit_depth = 10;
        flags = VPX_CODEC_USE_HIGHBITDEPTH;
        fmt = VPX_IMG_FMT_I42016;
    }
    vpx_codec_ctx_t enc;
    if (vpx_codec_enc_init(&enc, encoder, &cfg, flags) != VPX_CODEC_OK) {
        CHECK(0, "%s: the encoder refused to start: %s", label, vpx_codec_error(&enc));
        return;
    }
    vpx_codec_ctx_t dec;
    CHECK(vpx_codec_dec_init(&dec, decoder, NULL, 0) == VPX_CODEC_OK, "%s: the decoder refused to start", label);

    vpx_image_t *raw = vpx_img_alloc(NULL, fmt, WIDTH, HEIGHT, 1);
    if (bit_depth > 8) raw->bit_depth = 10;
    int decoded = 0;
    double worst = 99.0;
    for (int frame = 0; frame <= FRAMES; ++frame) {
        const int flushing = frame == FRAMES;
        if (!flushing) fill(raw, frame, bit_depth);
        CHECK(vpx_codec_encode(&enc, flushing ? NULL : raw, frame, 1, 0, VPX_DL_GOOD_QUALITY) == VPX_CODEC_OK,
              "%s: encoding frame %d failed: %s", label, frame, vpx_codec_error(&enc));
        vpx_codec_iter_t iter = NULL;
        const vpx_codec_cx_pkt_t *pkt;
        while ((pkt = vpx_codec_get_cx_data(&enc, &iter)) != NULL) {
            if (pkt->kind != VPX_CODEC_CX_FRAME_PKT) continue;
            CHECK(vpx_codec_decode(&dec, pkt->data.frame.buf, (unsigned int)pkt->data.frame.sz, NULL, 0) ==
                      VPX_CODEC_OK,
                  "%s: decoding a packet failed: %s", label, vpx_codec_error(&dec));
            vpx_codec_iter_t diter = NULL;
            vpx_image_t *img;
            while ((img = vpx_codec_get_frame(&dec, &diter)) != NULL) {
                CHECK(img->d_w == WIDTH && img->d_h == HEIGHT, "%s: picture %d is %ux%u", label, decoded, img->d_w,
                      img->d_h);
                CHECK((int)img->bit_depth == bit_depth, "%s: picture %d has %u bits", label, decoded, img->bit_depth);
                const double psnr = luma_psnr(img, decoded, bit_depth);
                if (psnr < worst) worst = psnr;
                ++decoded;
            }
        }
    }
    CHECK(decoded == FRAMES, "%s: %d pictures came back, expected %d", label, decoded, FRAMES);
    CHECK(worst >= 30.0, "%s: worst luma PSNR %.2f dB, expected at least 30", label, worst);
    printf("%s: %d frames encoded and decoded back, worst luma PSNR %.2f dB\n", label, decoded, worst);
    vpx_img_free(raw);
    vpx_codec_destroy(&enc);
    vpx_codec_destroy(&dec);
}

int main(void) {
    printf("libvpx %s (%s)\n", vpx_codec_version_str(), vpx_codec_build_config());
    CHECK(vpx_codec_version_major() >= 1, "library major version %d", vpx_codec_version_major());
    CHECK(strlen(vpx_codec_build_config()) > 0, "empty build configuration string");
    run("VP8", vpx_codec_vp8_cx(), vpx_codec_vp8_dx(), 8);
    run("VP9", vpx_codec_vp9_cx(), vpx_codec_vp9_dx(), 8);
    run("VP9 10-bit", vpx_codec_vp9_cx(), vpx_codec_vp9_dx(), 10);
    if (failures) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    printf("all checks passed\n");
    return 0;
}
