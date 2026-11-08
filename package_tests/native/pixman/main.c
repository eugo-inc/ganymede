/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pixman: core API; calls pixman_version_string(), pixman_image_create_bits(), pixman_image_fill_boxes(), pixman_image_unref().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pixman.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version string must be non-NULL and non-empty (documented API contract). */
    const char *v = pixman_version_string();
    assert(v != NULL);
    assert(strlen(v) > 0);

    uint32_t pixels[16] = {0};
    pixman_image_t *img = pixman_image_create_bits(PIXMAN_a8r8g8b8, 4, 4, pixels, 4 * sizeof(uint32_t));
    assert(img != NULL);

    pixman_color_t red = { 0xffff, 0x0000, 0x0000, 0xffff };
    pixman_box32_t box = { 0, 0, 4, 4 };
    pixman_bool_t ok = pixman_image_fill_boxes(PIXMAN_OP_SRC, img, &red, 1, &box);
    assert(ok);

    /* Verify the pixel buffer reflects opaque red (0xffff0000 in ARGB32). */
    for (int i = 0; i < 16; i++) {
        assert(pixels[i] == 0xffff0000u);
    }

    pixman_image_unref(img);

    printf("pixman test OK: %s\n", v);
    return 0;
}
