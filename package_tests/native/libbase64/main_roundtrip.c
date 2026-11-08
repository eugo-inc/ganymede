/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libbase64: roundtrip; calls base64_encode(), base64_decode(), base64_stream_encode_init(), base64_stream_encode().
 */

#include <libbase64.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* 0x00..0xff twice, so every byte value and every 3-byte alignment reaches the SIMD loop and the tail. */
    char plain[512];
    for (size_t i = 0; i < sizeof plain; i++) {
        plain[i] = (char)(unsigned char)(i & 0xff);
    }

    char encoded[1024];
    size_t encoded_len = 0;
    base64_encode(plain, sizeof plain, encoded, &encoded_len, 0);
    if (encoded_len != 684) { /* 4 * ceil(512 / 3) */
        fprintf(stderr, "base64_encode: length %zu, expected 684\n", encoded_len);
        return 1;
    }
    if (memcmp(encoded, "AAECAwQFBgc", 11) != 0) {
        fprintf(stderr, "base64_encode: unexpected prefix %.11s\n", encoded);
        return 1;
    }

    char decoded[512];
    size_t decoded_len = 0;
    if (base64_decode(encoded, encoded_len, decoded, &decoded_len, 0) != 1) {
        fprintf(stderr, "base64_decode: reported a decoding error on its own output\n");
        return 1;
    }
    if (decoded_len != sizeof plain || memcmp(decoded, plain, sizeof plain) != 0) {
        fprintf(stderr, "base64_decode: round trip mismatch (%zu bytes)\n", decoded_len);
        return 1;
    }

    /* Streaming encode in uneven chunks must give the one-shot bytes exactly. */
    struct base64_state state;
    char streamed[1024];
    size_t streamed_len = 0;
    size_t chunk_len = 0;
    base64_stream_encode_init(&state, 0);
    base64_stream_encode(&state, plain, 7, streamed, &chunk_len);
    streamed_len += chunk_len;
    base64_stream_encode(&state, plain + 7, sizeof plain - 7, streamed + streamed_len, &chunk_len);
    streamed_len += chunk_len;
    base64_stream_encode_final(&state, streamed + streamed_len, &chunk_len);
    streamed_len += chunk_len;
    if (streamed_len != encoded_len || memcmp(streamed, encoded, encoded_len) != 0) {
        fprintf(stderr, "base64_stream_encode: differs from base64_encode (%zu vs %zu bytes)\n", streamed_len, encoded_len);
        return 1;
    }

    size_t invalid_len = 0;
    if (base64_decode("QUJD*A==", 8, streamed, &invalid_len, 0) != 0) {
        fprintf(stderr, "base64_decode: accepted an invalid character\n");
        return 1;
    }

    printf("libbase64 round trip OK: %zu bytes -> %zu chars -> %zu bytes\n", sizeof plain, encoded_len, decoded_len);
    return 0;
}
