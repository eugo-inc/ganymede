/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openjpeg: extra options; calls opj_image_tile_create(), opj_image_destroy(), opj_image_create(), opj_set_default_encoder_parameters().
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
        cmptparms[i].h = 16;
        cmptparms[i].x0 = 0;
        cmptparms[i].y0 = 0;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = 0;
    }
    opj_image_t *tile_img = opj_image_tile_create(3, cmptparms, OPJ_CLRSPC_SRGB);
    assert(tile_img != NULL);
    assert(tile_img->numcomps == 3);
    assert(tile_img->color_space == OPJ_CLRSPC_SRGB);
    assert(tile_img->comps != NULL);
    assert(tile_img->comps[0].data == NULL);
    opj_image_destroy(tile_img);

    /* opj_encoder_set_extra_options — exercise after a real setup_encoder. */
    opj_image_t *img = opj_image_create(3, cmptparms, OPJ_CLRSPC_SRGB);
    assert(img != NULL);
    img->x0 = 0;
    img->y0 = 0;
    img->x1 = 16;
    img->y1 = 16;

    opj_cparameters_t cparams;
    opj_set_default_encoder_parameters(&cparams);

    opj_codec_t *enc = opj_create_compress(OPJ_CODEC_J2K);
    assert(enc != NULL);

    OPJ_BOOL ok = opj_setup_encoder(enc, &cparams, img);
    assert(ok == OPJ_TRUE);

    /* Valid options: NULL-terminated KEY=VALUE array. */
    const char *opts_good[] = {
        "PLT=YES",
        "GUARD_BITS=2",
        NULL,
    };
    ok = opj_encoder_set_extra_options(enc, opts_good);
    assert(ok == OPJ_TRUE);

    /* Malformed option must be rejected (unknown key). */
    const char *opts_bad[] = {
        "DEFINITELY_NOT_AN_OPENJPEG_OPTION=42",
        NULL,
    };
    ok = opj_encoder_set_extra_options(enc, opts_bad);
    assert(ok == OPJ_FALSE);

    opj_destroy_codec(enc);
    opj_image_destroy(img);

    printf("openjpeg extra-options test OK\n");
    return 0;
}
