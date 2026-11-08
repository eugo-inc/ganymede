/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpng: version; calls png_access_version_number(), png_create_write_struct(), png_create_info_struct(), png_get_libpng_ver().
 */

#include <png.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Compile-time macros from <png.h> */
    png_uint_32 ct_ver = PNG_LIBPNG_VER;
    const char *ct_ver_str = PNG_LIBPNG_VER_STRING;

    assert(ct_ver_str != NULL);
    assert(strlen(ct_ver_str) > 0);
    /* Major version 1; any 1.x release passes. */
    assert(ct_ver_str[0] == '1');
    assert(ct_ver_str[1] == '.');

    /* Runtime call into the .so */
    png_uint_32 rt_ver = png_access_version_number();
    assert(rt_ver == ct_ver);

    /* png_get_libpng_ver() returns the compiled-in string; needs a struct. */
    png_structp wp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(wp != NULL);
    png_infop wi = png_create_info_struct(wp);
    assert(wi != NULL);

    const char *runtime_libpng = png_get_libpng_ver(wp);
    const char *runtime_header = png_get_header_ver(wp);
    assert(runtime_libpng != NULL);
    assert(runtime_header != NULL);

    assert(strcmp(runtime_libpng, runtime_header) == 0);
    assert(strcmp(runtime_libpng, ct_ver_str) == 0);

    png_destroy_write_struct(&wp, &wi);

    printf("libpng version OK: ver=%u str=%s\n", (unsigned)rt_ver, ct_ver_str);
    return 0;
}
