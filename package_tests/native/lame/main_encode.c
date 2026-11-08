/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for lame: encode; calls get_lame_version(), lame_init(), lame_set_in_samplerate(), lame_set_num_channels().
 */

#include <lame/lame.h>

#include <stdio.h>

#define SAMPLE_COUNT 4608 /* four frames of 1152 */

int main(void) {
    static short pcm[SAMPLE_COUNT];
    static unsigned char mp3[SAMPLE_COUNT + SAMPLE_COUNT / 4 + 7200]; /* 1.25 * n + 7200: lame.h's worst case */
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        pcm[i] = (short)(((i * 37) % 2000) - 1000); /* a ramp; no libm needed */
    }

    const char *version = get_lame_version();
    if (version == NULL || version[0] == '\0') {
        fprintf(stderr, "get_lame_version returned nothing\n");
        return 1;
    }

    lame_global_flags *flags = lame_init();
    if (flags == NULL) {
        fprintf(stderr, "lame_init returned NULL\n");
        return 1;
    }
    lame_set_in_samplerate(flags, 44100);
    lame_set_num_channels(flags, 1);
    lame_set_mode(flags, MONO);
    if (lame_init_params(flags) < 0) {
        fprintf(stderr, "lame_init_params failed\n");
        lame_close(flags);
        return 1;
    }

    /* Mono: the right-channel buffer is ignored; pass the same one rather than NULL. */
    const int encoded = lame_encode_buffer(flags, pcm, pcm, SAMPLE_COUNT, mp3, (int)sizeof mp3);
    if (encoded < 0) {
        fprintf(stderr, "lame_encode_buffer returned %d\n", encoded);
        lame_close(flags);
        return 1;
    }
    const int flushed = lame_encode_flush(flags, mp3 + encoded, (int)sizeof mp3 - encoded);
    lame_close(flags);
    if (flushed < 0 || encoded + flushed <= 0) {
        fprintf(stderr, "lame produced no MP3 bytes (encode=%d flush=%d)\n", encoded, flushed);
        return 1;
    }

    hip_t decoder = hip_decode_init();
    if (decoder == NULL) {
        fprintf(stderr, "hip_decode_init returned NULL (LAME was built without its decoder)\n");
        return 1;
    }
    hip_decode_exit(decoder);

    printf("LAME %s: %d MP3 bytes from %d samples; decoder opened\n", version, encoded + flushed, SAMPLE_COUNT);
    return 0;
}
