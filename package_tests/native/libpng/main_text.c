/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpng: text; calls png_get_io_ptr(), png_create_write_struct(), png_create_info_struct(), setjmp().
 */

#include <png.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned char *data;
    size_t size;
    size_t capacity;
    size_t cursor;
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

static void mem_flush(png_structp png_ptr) { (void)png_ptr; }

static void mem_read(png_structp png_ptr, png_bytep data, png_size_t length) {
    mem_buf_t *b = (mem_buf_t *)png_get_io_ptr(png_ptr);
    assert(b->cursor + length <= b->size);
    memcpy(data, b->data + b->cursor, length);
    b->cursor += length;
}

int main(void) {
    /* --- Encode 1x1 grayscale + 2 tEXt chunks --- */
    mem_buf_t buf = {NULL, 0, 0, 0};
    png_structp wp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(wp != NULL);
    png_infop wi = png_create_info_struct(wp);
    assert(wi != NULL);

    if (setjmp(png_jmpbuf(wp))) {
        fprintf(stderr, "libpng error during write\n");
        return 1;
    }

    png_set_write_fn(wp, &buf, mem_write, mem_flush);
    png_set_IHDR(wp, wi, 1, 1, 8, PNG_COLOR_TYPE_GRAY,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    /* png_text storage must outlive the png_set_text() call; libpng copies
     * the strings during png_write_info(). */
    png_text texts[2];
    memset(texts, 0, sizeof(texts));

    char title_key[] = "Title";
    char title_val[] = "DemoNativeProbe";
    char author_key[] = "Author";
    char author_val[] = "libpng-test";

    texts[0].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[0].key = title_key;
    texts[0].text = title_val;
    texts[0].text_length = strlen(title_val);

    texts[1].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[1].key = author_key;
    texts[1].text = author_val;
    texts[1].text_length = strlen(author_val);

    png_set_text(wp, wi, texts, 2);
    png_write_info(wp, wi);

    unsigned char gray_px = 0x7F;
    png_bytep row = &gray_px;
    png_write_image(wp, &row);
    png_write_end(wp, wi);
    png_destroy_write_struct(&wp, &wi);

    assert(buf.size > 8);

    /* --- Decode and verify the tEXt chunks survive --- */
    buf.cursor = 0;
    png_structp rp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(rp != NULL);
    png_infop ri = png_create_info_struct(rp);
    assert(ri != NULL);

    if (setjmp(png_jmpbuf(rp))) {
        fprintf(stderr, "libpng error during read\n");
        return 1;
    }

    png_set_read_fn(rp, &buf, mem_read);
    png_read_info(rp, ri);

    /* Need to actually consume the IDAT to populate post-IDAT info; texts
     * before IDAT are available right after png_read_info. */
    unsigned char dec_px = 0;
    png_bytep dec_row = &dec_px;
    png_read_image(rp, &dec_row);
    png_read_end(rp, ri);

    png_textp out_text = NULL;
    int num_text = 0;
    png_uint_32 got = png_get_text(rp, ri, &out_text, &num_text);
    assert(got > 0);
    assert(num_text >= 2);
    assert(out_text != NULL);

    int found_title = 0, found_author = 0;
    for (int i = 0; i < num_text; i++) {
        assert(out_text[i].key != NULL);
        assert(out_text[i].text != NULL);
        if (strcmp(out_text[i].key, "Title") == 0) {
            assert(strcmp(out_text[i].text, "DemoNativeProbe") == 0);
            found_title = 1;
        } else if (strcmp(out_text[i].key, "Author") == 0) {
            assert(strcmp(out_text[i].text, "libpng-test") == 0);
            found_author = 1;
        }
    }
    assert(found_title);
    assert(found_author);

    png_destroy_read_struct(&rp, &ri, NULL);

    free(buf.data);
    printf("libpng tEXt chunk round-trip OK (Title + Author preserved)\n");
    return 0;
}
