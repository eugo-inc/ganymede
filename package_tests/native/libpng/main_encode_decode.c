/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpng: encode decode; calls png_get_io_ptr(), png_create_write_struct(), png_create_info_struct(), setjmp().
 */

#include <png.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMG_W 4
#define IMG_H 4
#define BYTES_PER_PIXEL 4  /* RGBA */
#define ROW_BYTES (IMG_W * BYTES_PER_PIXEL)

typedef struct {
    unsigned char *data;
    size_t size;
    size_t capacity;
    size_t cursor;  /* read cursor for the read path */
} mem_buf_t;

static void mem_write(png_structp png_ptr, png_bytep data, png_size_t length) {
    mem_buf_t *b = (mem_buf_t *)png_get_io_ptr(png_ptr);
    if (b->size + length > b->capacity) {
        size_t new_cap = (b->capacity == 0 ? 256 : b->capacity * 2);
        while (new_cap < b->size + length) new_cap *= 2;
        b->data = (unsigned char *)realloc(b->data, new_cap);
        assert(b->data != NULL);
        b->capacity = new_cap;
    }
    memcpy(b->data + b->size, data, length);
    b->size += length;
}

static void mem_flush(png_structp png_ptr) {
    (void)png_ptr;  /* no-op; everything stays in memory */
}

static void mem_read(png_structp png_ptr, png_bytep data, png_size_t length) {
    mem_buf_t *b = (mem_buf_t *)png_get_io_ptr(png_ptr);
    assert(b->cursor + length <= b->size);
    memcpy(data, b->data + b->cursor, length);
    b->cursor += length;
}

int main(void) {
    unsigned char src[IMG_H][ROW_BYTES];
    for (int y = 0; y < IMG_H; y++) {
        for (int x = 0; x < IMG_W; x++) {
            src[y][x * BYTES_PER_PIXEL + 0] = (unsigned char)(x * 60);        /* R */
            src[y][x * BYTES_PER_PIXEL + 1] = (unsigned char)(y * 60);        /* G */
            src[y][x * BYTES_PER_PIXEL + 2] = (unsigned char)((x + y) * 30);  /* B */
            src[y][x * BYTES_PER_PIXEL + 3] = 0xFF;                           /* A */
        }
    }

    /* --- Encode to memory --- */
    mem_buf_t enc = {NULL, 0, 0, 0};
    png_structp wp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(wp != NULL);
    png_infop wi = png_create_info_struct(wp);
    assert(wi != NULL);

    if (setjmp(png_jmpbuf(wp))) {
        fprintf(stderr, "libpng error during write\n");
        return 1;
    }

    png_set_write_fn(wp, &enc, mem_write, mem_flush);
    png_set_IHDR(wp, wi, IMG_W, IMG_H, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(wp, wi);

    png_bytep row_ptrs[IMG_H];
    for (int y = 0; y < IMG_H; y++) {
        row_ptrs[y] = src[y];
    }
    png_write_image(wp, row_ptrs);
    png_write_end(wp, NULL);
    png_destroy_write_struct(&wp, &wi);

    assert(enc.size > 8);  /* at least an 8-byte PNG signature got emitted */

    /* PNG signature is fixed: 89 50 4E 47 0D 0A 1A 0A */
    const unsigned char sig[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    assert(memcmp(enc.data, sig, 8) == 0);

    /* --- Decode from memory --- */
    enc.cursor = 0;
    png_structp rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(rp != NULL);
    png_infop ri = png_create_info_struct(rp);
    assert(ri != NULL);

    if (setjmp(png_jmpbuf(rp))) {
        fprintf(stderr, "libpng error during read\n");
        return 1;
    }

    png_set_read_fn(rp, &enc, mem_read);
    png_read_info(rp, ri);

    png_uint_32 dw = png_get_image_width(rp, ri);
    png_uint_32 dh = png_get_image_height(rp, ri);
    int bit_depth = png_get_bit_depth(rp, ri);
    int color_type = png_get_color_type(rp, ri);

    assert(dw == IMG_W);
    assert(dh == IMG_H);
    assert(bit_depth == 8);
    assert(color_type == PNG_COLOR_TYPE_RGBA);

    unsigned char dec[IMG_H][ROW_BYTES];
    png_bytep dec_rows[IMG_H];
    for (int y = 0; y < IMG_H; y++) {
        dec_rows[y] = dec[y];
    }
    png_read_image(rp, dec_rows);
    png_read_end(rp, NULL);
    png_destroy_read_struct(&rp, &ri, NULL);

    for (int y = 0; y < IMG_H; y++) {
        assert(memcmp(src[y], dec[y], ROW_BYTES) == 0);
    }

    free(enc.data);
    printf("libpng encode/decode round-trip OK (%dx%d RGBA, %zu encoded bytes)\n",
           IMG_W, IMG_H, enc.size);
    return 0;
}
