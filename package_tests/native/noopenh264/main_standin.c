/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for noopenh264: stub behaviour (encoder and decoder creation fail); calls WelsGetCodecVersionEx(), WelsCreateSVCEncoder(), WelsDestroySVCEncoder(), WelsCreateDecoder().
 */

#include <wels/codec_api.h>
#include <wels/codec_ver.h>

#include <stdio.h>

int main(void) {
    OpenH264Version version;
    WelsGetCodecVersionEx(&version);
    if (version.uMajor != OPENH264_MAJOR || version.uMinor != OPENH264_MINOR ||
        version.uRevision != OPENH264_REVISION) {
        fprintf(stderr, "codec_ver.h says %d.%d.%d but the library says %u.%u.%u\n", OPENH264_MAJOR, OPENH264_MINOR,
                OPENH264_REVISION, version.uMajor, version.uMinor, version.uRevision);
        return 1;
    }

    ISVCEncoder *encoder = NULL;
    const int encoder_rc = WelsCreateSVCEncoder(&encoder);
    if (encoder_rc == 0 || encoder != NULL) {
        fprintf(stderr, "WelsCreateSVCEncoder SUCCEEDED (rc=%d): this libopenh264 is a working encoder, not the "
                        "noopenh264 stub\n",
                encoder_rc);
        if (encoder != NULL) {
            WelsDestroySVCEncoder(encoder);
        }
        return 1;
    }

    ISVCDecoder *decoder = NULL;
    const long decoder_rc = WelsCreateDecoder(&decoder);
    if (decoder_rc == 0 || decoder != NULL) {
        fprintf(stderr, "WelsCreateDecoder SUCCEEDED (rc=%ld): this libopenh264 is a working decoder, not the "
                        "noopenh264 stub\n",
                decoder_rc);
        if (decoder != NULL) {
            WelsDestroyDecoder(decoder);
        }
        return 1;
    }

    printf("noopenh264 %u.%u.%u: stub behaviour confirmed (encoder rc=%d, decoder rc=%ld)\n", version.uMajor,
           version.uMinor, version.uRevision, encoder_rc, decoder_rc);
    return 0;
}
