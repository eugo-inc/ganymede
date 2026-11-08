/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpng: core API; calls png_create_read_struct(), png_create_info_struct(), png_destroy_read_struct(), png_create_write_struct().
 */

#include <png.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    png_structp read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(read_ptr != NULL);

    png_infop info_ptr = png_create_info_struct(read_ptr);
    assert(info_ptr != NULL);

    png_destroy_read_struct(&read_ptr, &info_ptr, NULL);

    png_structp write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(write_ptr != NULL);
    png_destroy_write_struct(&write_ptr, NULL);

    const char *v = png_libpng_ver;
    assert(v != NULL);
    assert(strlen(v) > 0);

    printf("libpng test OK: %s\n", v);
    return 0;
}
