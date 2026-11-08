/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openjpeg: encoder setup; calls opj_image_create(), opj_set_default_encoder_parameters(), opj_create_compress(), opj_setup_encoder().
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
        cmptparms[i].w = 32;
        cmptparms[i].h = 32;
        cmptparms[i].x0 = 0;
        cmptparms[i].y0 = 0;
        cmptparms[i].prec = 8;
        cmptparms[i].sgnd = 0;
    }
    opj_image_t *img = opj_image_create(3, cmptparms, OPJ_CLRSPC_SRGB);
    assert(img != NULL);
    img->x0 = 0;
    img->y0 = 0;
    img->x1 = 32;
    img->y1 = 32;

    opj_cparameters_t cparams;
    opj_set_default_encoder_parameters(&cparams);
    cparams.tile_size_on = OPJ_TRUE;
    cparams.cp_tdx = 16;
    cparams.cp_tdy = 16;
    cparams.tcp_numlayers = 2;
    cparams.cp_disto_alloc = 1;
    cparams.tcp_rates[0] = 4.0f;   /* layer 0: 4:1 rate */
    cparams.tcp_rates[1] = 1.0f;   /* layer 1: lossless */
    cparams.irreversible = 1;       /* lossy 9-7 DWT */
    cparams.prog_order = OPJ_RLCP;
    cparams.numresolution = 3;
    assert(cparams.cblockw_init > 0);
    assert(cparams.cblockh_init > 0);

    opj_codec_t *enc = opj_create_compress(OPJ_CODEC_J2K);
    assert(enc != NULL);
    OPJ_BOOL ok = opj_setup_encoder(enc, &cparams, img);
    assert(ok == OPJ_TRUE);
    opj_destroy_codec(enc);

    /* opj_set_MCT — identity matrix + zero DC shift on 3 components. */
    OPJ_FLOAT32 mct_matrix[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };
    OPJ_INT32 dc_shift[3] = {0, 0, 0};
    opj_cparameters_t cparams2;
    opj_set_default_encoder_parameters(&cparams2);
    cparams2.tcp_mct = 0;  /* MCT must be off before opj_set_MCT */
    ok = opj_set_MCT(&cparams2, mct_matrix, dc_shift, 3);
    assert(ok == OPJ_TRUE);

    opj_image_destroy(img);

    printf("openjpeg encoder-setup test OK\n");
    return 0;
}
