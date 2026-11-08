/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openjpeg: core API; calls opj_version(), opj_create_compress(), opj_set_default_encoder_parameters(), opj_destroy_codec().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openjpeg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version string must be present (documented contract). */
    const char *v = opj_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    opj_codec_t *enc = opj_create_compress(OPJ_CODEC_J2K);
    assert(enc != NULL);

    /* Default encoder parameters must populate without error. */
    opj_cparameters_t cparams;
    opj_set_default_encoder_parameters(&cparams);
    assert(cparams.numresolution > 0);

    opj_destroy_codec(enc);

    opj_codec_t *dec = opj_create_decompress(OPJ_CODEC_JP2);
    assert(dec != NULL);

    opj_dparameters_t dparams;
    opj_set_default_decoder_parameters(&dparams);

    opj_destroy_codec(dec);

    printf("openjpeg test OK: %s\n", v);
    return 0;
}
