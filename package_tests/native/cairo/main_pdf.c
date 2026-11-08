/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cairo: pdf; calls cairo_pdf_surface_create_for_stream(), cairo_surface_status(), cairo_create(), cairo_status().
 */

#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned char *data;
    unsigned int length;
    unsigned int capacity;
} pdf_buffer_t;

static cairo_status_t append_pdf_chunk(void *closure, const unsigned char *data, unsigned int length) {
    pdf_buffer_t *buf = (pdf_buffer_t *)closure;
    if (buf->length + length > buf->capacity) {
        unsigned int new_cap = (buf->capacity == 0) ? 4096 : buf->capacity * 2;
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
    pdf_buffer_t buf = {NULL, 0, 0};

    /* A4-ish dimensions in points; values don't matter as long as the backend accepts them. */
    cairo_surface_t *surface = cairo_pdf_surface_create_for_stream(append_pdf_chunk, &buf, 200.0, 200.0);
    assert(surface != NULL);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);

    cairo_t *cr = cairo_create(surface);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);

    cairo_set_source_rgb(cr, 0.2, 0.4, 0.6);
    cairo_rectangle(cr, 10.0, 10.0, 80.0, 60.0);
    cairo_fill(cr);
    assert(cairo_status(cr) == CAIRO_STATUS_SUCCESS);
    cairo_show_page(cr);

    cairo_destroy(cr);
    /* Finishing the surface flushes the trailing PDF structures into the buffer. */
    cairo_surface_finish(surface);
    assert(cairo_surface_status(surface) == CAIRO_STATUS_SUCCESS);
    cairo_surface_destroy(surface);

    /* Inspect the buffer. */
    assert(buf.data != NULL);
    assert(buf.length >= 16);

    /* PDF magic: "%PDF-" at the start. */
    static const unsigned char pdf_magic[5] = {'%', 'P', 'D', 'F', '-'};
    assert(memcmp(buf.data, pdf_magic, 5) == 0);

    /* PDFs end with "%%EOF" near the end of the file. */
    int found_eof = 0;
    /* Search the last 32 bytes (or the full buffer if smaller). */
    unsigned int search_start = (buf.length > 32) ? (buf.length - 32) : 0;
    for (unsigned int i = search_start; i + 5 <= buf.length; ++i) {
        if (memcmp(buf.data + i, "%%EOF", 5) == 0) {
            found_eof = 1;
            break;
        }
    }
    assert(found_eof);

    printf("cairo PDF test OK: %u bytes\n", buf.length);

    free(buf.data);
    return 0;
}
