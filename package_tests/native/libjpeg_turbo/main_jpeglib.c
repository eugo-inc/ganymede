/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libjpeg_turbo: jpeglib; calls longjmp(), jpeg_std_error(), setjmp(), jpeg_create_compress().
 */

#include <stddef.h>
#include <stdio.h>
#include <jpeglib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

struct my_err_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf jb;
};

static void my_error_exit(j_common_ptr cinfo) {
    struct my_err_mgr *err = (struct my_err_mgr *)cinfo->err;
    longjmp(err->jb, 1);
}

int main(void) {
    enum { W = 32, H = 24 };
    unsigned char src[W * H * 3];
    for (int i = 0; i < W * H * 3; i++) {
        src[i] = (unsigned char)((i * 7) & 0xFF);
    }

    /* --- Compress with classic libjpeg API --- */
    struct jpeg_compress_struct cinfo;
    struct my_err_mgr cerr;
    cinfo.err = jpeg_std_error(&cerr.pub);
    cerr.pub.error_exit = my_error_exit;
    if (setjmp(cerr.jb)) {
        fprintf(stderr, "jpeg compress error\n");
        return 1;
    }
    jpeg_create_compress(&cinfo);

    unsigned char *jpeg_buf = NULL;
    unsigned long jpeg_size = 0;
    jpeg_mem_dest(&cinfo, &jpeg_buf, &jpeg_size);

    cinfo.image_width = W;
    cinfo.image_height = H;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 85, TRUE);

    jpeg_start_compress(&cinfo, TRUE);
    JSAMPROW row[1];
    while (cinfo.next_scanline < cinfo.image_height) {
        row[0] = &src[cinfo.next_scanline * W * 3];
        jpeg_write_scanlines(&cinfo, row, 1);
    }
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    assert(jpeg_buf != NULL);
    assert(jpeg_size > 0);
    assert(jpeg_buf[0] == 0xFF);
    assert(jpeg_buf[1] == 0xD8);

    /* --- Decompress with classic libjpeg API --- */
    struct jpeg_decompress_struct dinfo;
    struct my_err_mgr derr;
    dinfo.err = jpeg_std_error(&derr.pub);
    derr.pub.error_exit = my_error_exit;
    if (setjmp(derr.jb)) {
        fprintf(stderr, "jpeg decompress error\n");
        free(jpeg_buf);
        return 1;
    }
    jpeg_create_decompress(&dinfo);
    jpeg_mem_src(&dinfo, jpeg_buf, jpeg_size);

    int rc = jpeg_read_header(&dinfo, TRUE);
    assert(rc == JPEG_HEADER_OK);
    assert(dinfo.image_width == W);
    assert(dinfo.image_height == H);
    assert(dinfo.num_components == 3);

    jpeg_start_decompress(&dinfo);
    assert(dinfo.output_width == W);
    assert(dinfo.output_height == H);
    assert(dinfo.output_components == 3);

    /* Drain all scanlines to fully exercise the decoder. */
    int row_stride = dinfo.output_width * dinfo.output_components;
    unsigned char *line = (unsigned char *)malloc(row_stride);
    assert(line != NULL);
    while (dinfo.output_scanline < dinfo.output_height) {
        JSAMPROW r[1] = { line };
        int n = jpeg_read_scanlines(&dinfo, r, 1);
        assert(n == 1);
    }
    free(line);
    jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);

    free(jpeg_buf);
    printf("libjpeg_turbo jpeglib API OK\n");
    return 0;
}
