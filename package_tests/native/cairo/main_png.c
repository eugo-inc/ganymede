/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cairo: png; calls cairo_image_surface_create(), cairo_surface_status(), cairo_create(), cairo_set_source_rgb().
 */

#include <cairo/cairo.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned char *data;
    unsigned int length;
    unsigned int capacity;
} png_buffer_t;

static cairo_status_t append_png_chunk(void *closure, const unsigned char *data, unsigned int length) {
    png_buffer_t *buf = (png_buffer_t *)closure;
    if (buf->length + length > buf->capacity) {
        unsigned int new_cap = (buf->capacity == 0) ? 1024 : buf->capacity * 2;
        while (new_cap < buf->length + length) new_cap *= 2;
        unsigned char *grown = (unsigned char *)realloc(buf->data, new_cap);
        if (!grown) return CAIRO_STATUS_WRITE_ERROR;
        buf->data = grown;
        buf->capacity = new_cap;
    }
    memcpy(buf->data + buf->length, data, length);
    buf->length += length;
    return CAIRO_STATUS_SUCCESS;
}

int main(void) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 8, 8);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);

    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0.25, 0.5, 0.75);
    cairo_paint(cr);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);
    cairo_destroy(cr);

    png_buffer_t buf = {NULL, 0, 0};
    cairo_status_t st = cairo_surface_write_to_png_stream(surface, append_png_chunk, &buf);
    assert(st == CAIRO_STATUS_SUCCESS);
    assert(buf.data != NULL);
    assert(buf.length >= 8);

    /* PNG magic: 89 50 4E 47 0D 0A 1A 0A */
    static const unsigned char png_magic[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    assert(memcmp(buf.data, png_magic, 8) == 0);

    /* PNGs end with an IEND chunk; sanity-check the buffer contains the IEND ascii. */
    int found_iend = 0;
    for (unsigned int i = 0; i + 4 <= buf.length; ++i) {
        if (memcmp(buf.data + i, "IEND", 4) == 0) {
            found_iend = 1;
            break;
        }
    }
    assert(found_iend);

    printf("cairo PNG test OK: %u bytes\n", buf.length);

    free(buf.data);
    cairo_surface_destroy(surface);
    return 0;
}
