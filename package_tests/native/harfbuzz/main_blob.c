/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for harfbuzz: blob; calls hb_blob_create(), hb_blob_get_empty(), hb_blob_get_length(), hb_blob_get_data().
 */

#include <hb.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char payload[] = "demo-harfbuzz-blob-probe";
    const unsigned int payload_len = (unsigned int) (sizeof(payload) - 1);

    /* Create a read-only blob referencing static storage. */
    hb_blob_t *blob = hb_blob_create(
        payload, payload_len, HB_MEMORY_MODE_READONLY, NULL, NULL);
    assert(blob != NULL);
    assert(blob != hb_blob_get_empty());

    assert(hb_blob_get_length(blob) == payload_len);

    unsigned int got_len = 0;
    const char *got = hb_blob_get_data(blob, &got_len);
    assert(got != NULL);
    assert(got_len == payload_len);
    assert(memcmp(got, payload, payload_len) == 0);

    hb_blob_destroy(blob);

    /* Empty-blob singleton: length 0, non-NULL, idempotent. */
    hb_blob_t *empty = hb_blob_get_empty();
    assert(empty != NULL);
    assert(hb_blob_get_length(empty) == 0);
    assert(hb_blob_get_empty() == empty);

    /* Sub-blob: take last 4 bytes ("robe"), verify length + content. */
    hb_blob_t *blob2 = hb_blob_create(
        payload, payload_len, HB_MEMORY_MODE_READONLY, NULL, NULL);
    hb_blob_t *sub = hb_blob_create_sub_blob(blob2, payload_len - 4, 4);
    assert(sub != NULL);
    assert(hb_blob_get_length(sub) == 4);
    unsigned int sub_len = 0;
    const char *sub_data = hb_blob_get_data(sub, &sub_len);
    assert(sub_len == 4);
    assert(memcmp(sub_data, payload + payload_len - 4, 4) == 0);
    hb_blob_destroy(sub);
    hb_blob_destroy(blob2);

    printf("harfbuzz blob test OK\n");
    return 0;
}
