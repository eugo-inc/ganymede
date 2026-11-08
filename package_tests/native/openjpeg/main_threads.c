/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openjpeg: threads; calls opj_has_thread_support(), opj_get_num_cpus(), opj_create_decompress(), opj_codec_set_threads().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openjpeg.h>
#include <assert.h>
#include <stdio.h>

static void on_info(const char *msg, void *user) {
    (void)msg;
    (void)user;
}

static void on_warning(const char *msg, void *user) {
    (void)msg;
    (void)user;
}

static void on_error(const char *msg, void *user) {
    (void)msg;
    (void)user;
}

int main(void) {
    OPJ_BOOL has_threads = opj_has_thread_support();
    assert(has_threads == OPJ_TRUE);

    int ncpus = opj_get_num_cpus();
    assert(ncpus > 0);

    /* Codec accepts a thread-count request. */
    opj_codec_t *dec = opj_create_decompress(OPJ_CODEC_J2K);
    assert(dec != NULL);

    OPJ_BOOL ok = opj_codec_set_threads(dec, 1);
    assert(ok == OPJ_TRUE);

    int marker = 0;
    ok = opj_set_info_handler(dec, on_info, &marker);
    assert(ok == OPJ_TRUE);
    ok = opj_set_warning_handler(dec, on_warning, &marker);
    assert(ok == OPJ_TRUE);
    ok = opj_set_error_handler(dec, on_error, &marker);
    assert(ok == OPJ_TRUE);

    opj_destroy_codec(dec);

    /* Strict-mode toggle on a fresh decoder (OpenJPEG 2.5 or newer). */
    opj_codec_t *dec2 = opj_create_decompress(OPJ_CODEC_JP2);
    assert(dec2 != NULL);
    ok = opj_decoder_set_strict_mode(dec2, OPJ_FALSE);
    assert(ok == OPJ_TRUE);
    opj_destroy_codec(dec2);

    printf("openjpeg threads+handlers test OK (cpus=%d)\n", ncpus);
    return 0;
}
