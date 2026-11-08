/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libvorbis: encode headers; calls vorbis_version_string(), vorbis_info_init(), vorbis_encode_init_vbr(), vorbis_info_clear().
 */

#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    const char *version = vorbis_version_string();
    if (version == NULL) {
        fprintf(stderr, "vorbis_version_string returned NULL\n");
        return 1;
    }

    vorbis_info info;
    vorbis_info_init(&info);
    if (vorbis_encode_init_vbr(&info, 1, 44100, 0.4f) != 0) {
        fprintf(stderr, "vorbis_encode_init_vbr failed\n");
        vorbis_info_clear(&info);
        return 1;
    }

    vorbis_comment comment;
    vorbis_comment_init(&comment);
    vorbis_comment_add_tag(&comment, "ENCODER", "demo-native-test");

    vorbis_dsp_state dsp;
    vorbis_block block;
    if (vorbis_analysis_init(&dsp, &info) != 0) {
        fprintf(stderr, "vorbis_analysis_init failed\n");
        return 1;
    }
    vorbis_block_init(&dsp, &block);

    ogg_packet identification;
    ogg_packet comments;
    ogg_packet codebooks;
    const int rc = vorbis_analysis_headerout(&dsp, &comment, &identification, &comments, &codebooks);
    const int ok = rc == 0 && identification.bytes > 7 && comments.bytes > 0 && codebooks.bytes > 0 &&
                   identification.packet[0] == 1 && memcmp(identification.packet + 1, "vorbis", 6) == 0;
    const long total = ok ? identification.bytes + comments.bytes + codebooks.bytes : 0;

    vorbis_block_clear(&block);
    vorbis_dsp_clear(&dsp);
    vorbis_comment_clear(&comment);
    vorbis_info_clear(&info);

    if (!ok) {
        fprintf(stderr, "vorbis_analysis_headerout rc=%d produced malformed header packets\n", rc);
        return 1;
    }

    printf("%s: three header packets, %ld bytes\n", version, total);
    return 0;
}
