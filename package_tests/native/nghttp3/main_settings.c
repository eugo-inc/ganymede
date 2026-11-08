/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for nghttp3: settings; calls nghttp3_settings_default().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <nghttp3/nghttp3.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    nghttp3_settings settings;

    /* Poison-fill, then init. If nghttp3_settings_default forgot to
     * memset, the poison would survive into the asserts below. */
    memset(&settings, 0xAA, sizeof settings);

    nghttp3_settings_default(&settings);

    assert(settings.max_field_section_size >= 4611686018427387903ULL);

    assert(settings.qpack_encoder_max_dtable_capacity == 4096);

    /* The remaining V1 fields must be cleared. */
    assert(settings.qpack_max_dtable_capacity == 0);
    assert(settings.qpack_blocked_streams == 0);
    assert(settings.enable_connect_protocol == 0);
    assert(settings.h3_datagram == 0);

    /* Second init on the same struct must produce identical state
     * (idempotency — the function must not OR-in to existing fields). */
    nghttp3_settings snapshot = settings;
    nghttp3_settings_default(&settings);
    assert(memcmp(&snapshot, &settings, sizeof settings) == 0);

    /* Sanity: independent inits of two structs must be byte-identical. */
    nghttp3_settings other;
    memset(&other, 0x55, sizeof other);
    nghttp3_settings_default(&other);
    assert(memcmp(&other, &settings, sizeof settings) == 0);

    printf("nghttp3 settings default test OK\n");
    return 0;
}
