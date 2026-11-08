/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for dav1d: decode av1; calls dav1d_default_settings(), dav1d_open(), dav1d_data_create(), dav1d_send_data().
 */

#include <dav1d/dav1d.h>

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

static uint64_t hash_picture(const Dav1dPicture *picture) {
    const int bytes_per_sample = picture->p.bpc > 8 ? 2 : 1;
    const int chroma_width = (picture->p.w + 1) >> 1;
    const int chroma_height = (picture->p.h + 1) >> 1;
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (int plane = 0; plane < 3; ++plane) {
        const int width = plane == 0 ? picture->p.w : chroma_width;
        const int height = plane == 0 ? picture->p.h : chroma_height;
        const ptrdiff_t stride = picture->stride[plane == 0 ? 0 : 1];
        const uint8_t *row = (const uint8_t *)picture->data[plane];
        for (int y = 0; y < height; ++y, row += stride) {
            hash = fnv1a_update(hash, row, (size_t)width * (size_t)bytes_per_sample);
        }
    }
    return hash;
}

static uint32_t read_le32(const uint8_t *bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

/* Returns the number of failed checks for one stream. */
static int decode_and_check(const struct demo_av1_sample *sample) {
    if (sample->ivf_size < 32 || memcmp(sample->ivf, "DKIF", 4) != 0) {
        fprintf(stderr, "FAIL [%s]: the compiled-in sample is not an IVF file\n", sample->name);
        return 1;
    }

    Dav1dSettings settings;
    dav1d_default_settings(&settings);
    settings.n_threads = 1;
    settings.max_frame_delay = 1;
    Dav1dContext *context = NULL;
    int result = dav1d_open(&context, &settings);
    if (result < 0) {
        fprintf(stderr, "FAIL [%s]: dav1d_open returned %d\n", sample->name, result);
        return 1;
    }

    int failures = 0;
    int frame_count = 0;
    const uint16_t header_size = (uint16_t)(sample->ivf[6] | (sample->ivf[7] << 8));
    size_t offset = header_size;
    int is_input_done = 0;
    while (failures == 0) {
        Dav1dData data = {0};
        if (!is_input_done) {
            if (offset + 12 > sample->ivf_size) {
                is_input_done = 1;
            } else {
                const uint32_t frame_size = read_le32(sample->ivf + offset);
                offset += 12;
                if (frame_size == 0 || offset + frame_size > sample->ivf_size) {
                    fprintf(stderr, "FAIL [%s]: truncated IVF frame at byte %zu\n", sample->name, offset);
                    failures += 1;
                    break;
                }
                uint8_t *buffer = dav1d_data_create(&data, frame_size);
                if (buffer == NULL) {
                    fprintf(stderr, "FAIL [%s]: dav1d_data_create(%u) returned NULL\n", sample->name, frame_size);
                    failures += 1;
                    break;
                }
                memcpy(buffer, sample->ivf + offset, frame_size);
                offset += frame_size;
            }
        }

        /* Feed the frame; while the decoder has a picture waiting it answers EAGAIN, so drain between sends. */
        do {
            if (data.sz > 0) {
                result = dav1d_send_data(context, &data);
                if (result < 0 && result != DAV1D_ERR(EAGAIN)) {
                    fprintf(stderr, "FAIL [%s]: dav1d_send_data returned %d\n", sample->name, result);
                    failures += 1;
                    break;
                }
            }
            Dav1dPicture picture = {0};
            while ((result = dav1d_get_picture(context, &picture)) == 0) {
                if (frame_count >= sample->frame_count) {
                    fprintf(stderr, "FAIL [%s]: decoded more than the %d expected frames\n", sample->name,
                            sample->frame_count);
                    failures += 1;
                } else if (picture.p.w != sample->width || picture.p.h != sample->height ||
                           picture.p.layout != DAV1D_PIXEL_LAYOUT_I420 || picture.p.bpc != sample->bits_per_sample) {
                    fprintf(stderr, "FAIL [%s]: frame %d is %dx%d layout %d at %d bits, expected %dx%d 4:2:0 at %d\n",
                            sample->name, frame_count, picture.p.w, picture.p.h, (int)picture.p.layout,
                            picture.p.bpc, sample->width, sample->height, sample->bits_per_sample);
                    failures += 1;
                } else {
                    const uint64_t hash = hash_picture(&picture);
                    if (hash != sample->frame_hashes[frame_count]) {
                        fprintf(stderr,
                                "FAIL [%s]: frame %d hashes to 0x%016" PRIx64 ", expected 0x%016" PRIx64 "\n",
                                sample->name, frame_count, hash, sample->frame_hashes[frame_count]);
                        failures += 1;
                    }
                }
                frame_count += 1;
                dav1d_picture_unref(&picture);
            }
            if (result != DAV1D_ERR(EAGAIN)) {
                fprintf(stderr, "FAIL [%s]: dav1d_get_picture returned %d\n", sample->name, result);
                failures += 1;
                break;
            }
        } while (data.sz > 0 && failures == 0);
        if (data.sz > 0) {
            dav1d_data_unref(&data);
        }
        if (is_input_done) {
            break;
        }
    }
    dav1d_close(&context);

    if (failures == 0 && frame_count != sample->frame_count) {
        fprintf(stderr, "FAIL [%s]: decoded %d frames, expected %d\n", sample->name, frame_count, sample->frame_count);
        failures += 1;
    }
    if (failures == 0) {
        printf("%s: %d frames of %dx%d at %d bits decoded; every frame matches the expected hashes\n", sample->name,
               frame_count, sample->width, sample->height, sample->bits_per_sample);
    }
    return failures;
}

int main(void) {
    int failures = 0;

    const unsigned api_version = dav1d_version_api();
    if (DAV1D_API_MAJOR(api_version) != DAV1D_API_VERSION_MAJOR ||
        DAV1D_API_MINOR(api_version) < DAV1D_API_VERSION_MINOR) {
        fprintf(stderr, "FAIL: libdav1d reports API %u.%u.%u, but this was compiled against the %d.%d headers\n",
                DAV1D_API_MAJOR(api_version), DAV1D_API_MINOR(api_version), DAV1D_API_PATCH(api_version),
                DAV1D_API_VERSION_MAJOR, DAV1D_API_VERSION_MINOR);
        failures += 1;
    } else {
        printf("libdav1d %s, API %u.%u.%u\n", dav1d_version(), DAV1D_API_MAJOR(api_version),
               DAV1D_API_MINOR(api_version), DAV1D_API_PATCH(api_version));
    }

    for (size_t i = 0; i < sizeof(demo_av1_samples) / sizeof(demo_av1_samples[0]); ++i) {
        failures += decode_and_check(&demo_av1_samples[i]);
    }

    if (failures != 0) {
        fprintf(stderr, "dav1d: %d check(s) failed\n", failures);
        return 2;
    }
    printf("dav1d OK: 8-bit and 10-bit AV1 decoded bit-exactly\n");
    return 0;
}
