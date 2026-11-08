/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libsoxr: core API; calls soxr_quality_spec(), soxr_create(), soxr_engine(), soxr_delete().
 */

#define _GNU_SOURCE /* M_PI under a strict -std= */

#include <soxr.h>

#include <math.h>
#include <stdio.h>

#define INPUT_RATE 44100.0
#define OUTPUT_RATE 16000.0
#define INPUT_FRAMES 4410

int main(void) {
    soxr_error_t error = NULL;
    const soxr_quality_spec_t quality = soxr_quality_spec(SOXR_HQ, 0);
    soxr_t resampler = soxr_create(INPUT_RATE, OUTPUT_RATE, 1, &error, NULL, &quality, NULL);
    if (resampler == NULL || error != NULL) {
        fprintf(stderr, "libsoxr: soxr_create failed: %s\n", error != NULL ? error : "(null)");
        return 1;
    }
    /* The engine libsoxr picks depends on the CPU and on how it was built; any named engine is fine. */
    const char *engine = soxr_engine(resampler);
    if (engine == NULL || engine[0] == '\0') {
        fprintf(stderr, "libsoxr: soxr_engine returned no engine name\n");
        return 1;
    }
    char engine_name[32];
    snprintf(engine_name, sizeof engine_name, "%s", engine);
    soxr_delete(resampler);

    static float input[INPUT_FRAMES];
    static float output[INPUT_FRAMES];
    for (int index = 0; index < INPUT_FRAMES; ++index) {
        input[index] = (float) sin(2.0 * M_PI * 1000.0 * (double) index / INPUT_RATE);
    }

    size_t input_done = 0;
    size_t output_done = 0;
    error = soxr_oneshot(INPUT_RATE, OUTPUT_RATE, 1, input, INPUT_FRAMES, &input_done, output, INPUT_FRAMES,
                         &output_done, NULL, &quality, NULL);
    if (error != NULL) {
        fprintf(stderr, "libsoxr: soxr_oneshot failed: %s\n", error);
        return 1;
    }
    if (input_done != INPUT_FRAMES || output_done < 1590 || output_done > 1610) {
        fprintf(stderr, "libsoxr: consumed %zu of %d frames, produced %zu (expected ~1600)\n", input_done, INPUT_FRAMES,
                output_done);
        return 1;
    }

    float peak = 0.0F;
    for (size_t index = output_done / 4; index < 3 * output_done / 4; ++index) {
        peak = fmaxf(peak, fabsf(output[index]));
    }
    if (peak < 0.9F || peak > 1.1F) {
        fprintf(stderr, "libsoxr: peak amplitude %f after resampling a unit sine\n", (double) peak);
        return 1;
    }

    printf("libsoxr %s: engine %s, 4410 -> %zu frames, peak %.3f\n", soxr_version(), engine_name, output_done,
           (double) peak);
    return 0;
}
