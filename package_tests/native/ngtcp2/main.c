/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ngtcp2: core API; calls ngtcp2_version(), ngtcp2_strerror(), ngtcp2_settings_default().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <ngtcp2/ngtcp2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const ngtcp2_info *info = ngtcp2_version(0);
    assert(info != NULL);
    assert(info->version_str != NULL);
    assert(strlen(info->version_str) > 0);

    /* NGTCP2_ERR_INVALID_ARGUMENT is part of ngtcp2's stable public error enum.
     * strerror must return a non-empty string for it. */
    const char *msg = ngtcp2_strerror(NGTCP2_ERR_INVALID_ARGUMENT);
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    ngtcp2_settings settings;
    memset(&settings, 0, sizeof(settings));
    ngtcp2_settings_default(&settings);
    assert(settings.max_tx_udp_payload_size > 0);

    printf("ngtcp2 test OK: %s (max_tx_udp_payload_size=%zu)\n",
           info->version_str, (size_t)settings.max_tx_udp_payload_size);
    return 0;
}
