/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openjpeg: image; calls opj_image_create(), opj_image_data_alloc(), opj_image_data_free(), opj_image_destroy().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openjpeg.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    opj_image_cmptparm_t cmptparms[3];
    memset(cmptparms, 0, sizeof(cmptparms));
    for (int i = 0; i < 3; i++) {
        cmptparms[i].dx = 1;
        cmptparms[i].dy = 1;
        cmptparms[i].w = 16;
        cmptparms[i].h = 8;
        cmptparms[i].x0 = 0;
        cmptparms[i].y0 = 0;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = 0;
    }

    opj_image_t *img = opj_image_create(3, cmptparms, OPJ_CLRSPC_SRGB);
    assert(img != NULL);
    assert(img->numcomps == 3);
    assert(img->color_space == OPJ_CLRSPC_SRGB);
    assert(img->comps != NULL);

    /* Verify component table received the requested geometry. */
    for (OPJ_UINT32 i = 0; i < img->numcomps; i++) {
        assert(img->comps[i].dx == 1);
        assert(img->comps[i].dy == 1);
        assert(img->comps[i].w == 16);
        assert(img->comps[i].h == 8);
        assert(img->comps[i].prec == 8);
    }

    OPJ_SIZE_T nbytes = 16 * 8 * sizeof(OPJ_INT32);
    void *buf = opj_image_data_alloc(nbytes);
    assert(buf != NULL);
    /* Write + read back to confirm the allocation is writable. */
    memset(buf, 0xAB, nbytes);
    assert(((unsigned char *)buf)[0] == 0xAB);
    assert(((unsigned char *)buf)[nbytes - 1] == 0xAB);
    opj_image_data_free(buf);

    opj_image_destroy(img);

    printf("openjpeg image-model test OK\n");
    return 0;
}
