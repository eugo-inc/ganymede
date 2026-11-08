/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtiff: compression; calls mkstemps(), TIFFOpen(), TIFFSetField(), TIFFWriteEncodedStrip().
 */

#include <tiffio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define CHECK(condition)                                                                      \
    do {                                                                                      \
        if (!(condition)) {                                                                   \
            fprintf(stderr, "libtiff compression test FAILED at line %d: %s\n", __LINE__, #condition); \
            exit(1);                                                                          \
        }                                                                                     \
    } while (0)

struct codec {
    uint16_t scheme;
    const char *name;
};

/* The probe needs Deflate. The other codecs are optional in libtiff; the ones this libtiff lacks are reported and
 * skipped. */
static const struct codec EXPECTED_CODECS[] = {
    {COMPRESSION_JBIG, "JBIG"},
    {COMPRESSION_JPEG, "JPEG"},
    {COMPRESSION_LERC, "LERC"},
    {COMPRESSION_ADOBE_DEFLATE, "Deflate"},
    {COMPRESSION_LZMA, "LZMA"},
    {COMPRESSION_ZSTD, "ZSTD"},
    {COMPRESSION_WEBP, "WebP"},
};

static void round_trip(uint16_t scheme, const char *name) {
    char path[] = "/tmp/demo_libtiff_codec_XXXXXX.tif";
    int fd = mkstemps(path, 4);
    CHECK(fd >= 0);
    close(fd);

    const uint32_t W = 64, H = 64;
    const size_t N = (size_t)W * H * 3;
    unsigned char *src = (unsigned char *)malloc(N);
    CHECK(src != NULL);
    /* Highly compressible payload so the size check is meaningful. */
    memset(src, 0xA5, N);

    TIFF *out = TIFFOpen(path, "w");
    CHECK(out != NULL);
    CHECK(TIFFSetField(out, TIFFTAG_IMAGEWIDTH, W) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_IMAGELENGTH, H) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, 8) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, 3) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, H) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 1);
    CHECK(TIFFSetField(out, TIFFTAG_COMPRESSION, scheme) == 1);
    if (scheme == COMPRESSION_WEBP) {
        /* WebP is lossy unless asked otherwise; the byte comparison needs lossless. */
        CHECK(TIFFSetField(out, TIFFTAG_WEBP_LOSSLESS, 1) == 1);
    }
    CHECK(TIFFWriteEncodedStrip(out, 0, src, N) == (tmsize_t)N);
    TIFFClose(out);

    struct stat st;
    CHECK(stat(path, &st) == 0);
    if (!((size_t)st.st_size < N / 2)) {
        fprintf(stderr, "libtiff compression test FAILED: %s wrote %lld bytes for a %zu-byte payload\n", name,
                (long long)st.st_size, N);
        exit(1);
    }

    TIFF *in = TIFFOpen(path, "r");
    CHECK(in != NULL);
    uint16_t compression = 0;
    CHECK(TIFFGetField(in, TIFFTAG_COMPRESSION, &compression) == 1);
    CHECK(compression == scheme);
    unsigned char *back = (unsigned char *)malloc(N);
    CHECK(back != NULL);
    CHECK(TIFFReadEncodedStrip(in, 0, back, N) == (tmsize_t)N);
    if (memcmp(src, back, N) != 0) {
        fprintf(stderr, "libtiff compression test FAILED: %s did not give the same bytes back\n", name);
        exit(1);
    }
    TIFFClose(in);

    unlink(path);
    free(src);
    free(back);
    printf("libtiff %s round-trip OK\n", name);
}

int main(void) {
    if (!TIFFIsCODECConfigured(COMPRESSION_ADOBE_DEFLATE)) {
        fprintf(stderr, "libtiff compression test FAILED: the Deflate codec is not available in this libtiff\n");
        return 1;
    }
    for (size_t index = 0; index < sizeof(EXPECTED_CODECS) / sizeof(EXPECTED_CODECS[0]); ++index) {
        if (!TIFFIsCODECConfigured(EXPECTED_CODECS[index].scheme)) {
            printf("libtiff note: the %s codec is not built into this libtiff, skipped\n", EXPECTED_CODECS[index].name);
        }
    }

    static const struct codec ROUND_TRIP_CODECS[] = {
        {COMPRESSION_ADOBE_DEFLATE, "Deflate"},
        {COMPRESSION_ZSTD, "ZSTD"},
        {COMPRESSION_WEBP, "WebP"},
        {COMPRESSION_LERC, "LERC"},
    };
    for (size_t index = 0; index < sizeof(ROUND_TRIP_CODECS) / sizeof(ROUND_TRIP_CODECS[0]); ++index) {
        if (TIFFIsCODECConfigured(ROUND_TRIP_CODECS[index].scheme)) {
            round_trip(ROUND_TRIP_CODECS[index].scheme, ROUND_TRIP_CODECS[index].name);
        }
    }

    printf("libtiff compression test OK\n");
    return 0;
}
