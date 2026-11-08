/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libsndfile: core API; calls sf_open(), sf_strerror(), sf_writef_float(), sf_close().
 */

#define _GNU_SOURCE /* M_PI and mkstemp under a strict -std= */

#include <sndfile.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FRAMES 4800
/* The sample check looks at 2400 frames from frame 1200: exactly 22 periods of 440 Hz at 48 kHz, well clear of the
 * edges where a lossy codec's encoder delay and padding sit. */
#define WINDOW_START 1200
#define WINDOW_FRAMES 2400
#define TONE_HZ 440.0

/* Opus (libsndfile 1.0.29) and MPEG audio (1.1.0) are enum constants, which the preprocessor cannot test, so the
 * probe names them by value and still compiles against older headers. codec_available() decides at run time. */
#define PROBE_FORMAT_OPUS 0x0064
#define PROBE_FORMAT_MPEG 0x230000
#define PROBE_FORMAT_MPEG_LAYER_III 0x0082

typedef struct {
    const char *name;
    int format;
    int sample_rate;
    int required;
} codec_case;

/* 1 when the format list read with count_command and item_command (major formats or subtypes) contains format. */
static int listed(int count_command, int item_command, int format) {
    int count = 0;
    if (sf_command(NULL, count_command, &count, (int) sizeof count) != 0) {
        return 0;
    }
    for (int index = 0; index < count; ++index) {
        SF_FORMAT_INFO info;
        memset(&info, 0, sizeof info);
        info.format = index;
        if (sf_command(NULL, item_command, &info, (int) sizeof info) == 0 && info.format == format) {
            return 1;
        }
    }
    return 0;
}

/* sf_format_check() only says whether a combination is valid. The major-format and subtype lists hold only the codecs
 * this libsndfile was built with, so a codec missing from either one is not available. */
static int codec_available(const codec_case *codec) {
    SF_INFO info;
    memset(&info, 0, sizeof info);
    info.samplerate = codec->sample_rate;
    info.channels = 1;
    info.format = codec->format;
    return sf_format_check(&info) &&
           listed(SFC_GET_FORMAT_MAJOR_COUNT, SFC_GET_FORMAT_MAJOR, codec->format & SF_FORMAT_TYPEMASK) &&
           listed(SFC_GET_FORMAT_SUBTYPE_COUNT, SFC_GET_FORMAT_SUBTYPE, codec->format & SF_FORMAT_SUBMASK);
}

static int round_trip(const codec_case *codec) {
    static float samples[FRAMES];
    static float readback[FRAMES * 2];
    for (int index = 0; index < FRAMES; ++index) {
        samples[index] = 0.5F * (float) sin(2.0 * M_PI * 440.0 * (double) index / (double) codec->sample_rate);
    }

    char path[] = "/tmp/demo_libsndfile_probe_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) {
        perror("mkstemp");
        return 1;
    }
    close(fd);

    SF_INFO write_info;
    memset(&write_info, 0, sizeof write_info);
    write_info.samplerate = codec->sample_rate;
    write_info.channels = 1;
    write_info.format = codec->format;
    SNDFILE *writer = sf_open(path, SFM_WRITE, &write_info);
    if (writer == NULL) {
        fprintf(stderr, "libsndfile: %s: cannot open for WRITE: %s (codec not compiled in?)\n", codec->name,
                sf_strerror(NULL));
        unlink(path);
        return 1;
    }
    if (sf_writef_float(writer, samples, FRAMES) != FRAMES) {
        fprintf(stderr, "libsndfile: %s: short write: %s\n", codec->name, sf_strerror(writer));
        sf_close(writer);
        unlink(path);
        return 1;
    }
    sf_close(writer);

    SF_INFO read_info;
    memset(&read_info, 0, sizeof read_info);
    SNDFILE *reader = sf_open(path, SFM_READ, &read_info);
    unlink(path);
    if (reader == NULL) {
        fprintf(stderr, "libsndfile: %s: cannot open for READ: %s\n", codec->name, sf_strerror(NULL));
        return 1;
    }
    const sf_count_t frames_read = sf_readf_float(reader, readback, FRAMES * 2);
    sf_close(reader);

    if ((read_info.format & SF_FORMAT_TYPEMASK) != (codec->format & SF_FORMAT_TYPEMASK) ||
        (read_info.format & SF_FORMAT_SUBMASK) != (codec->format & SF_FORMAT_SUBMASK) || read_info.channels != 1 ||
        frames_read < FRAMES * 9 / 10) {
        fprintf(stderr, "libsndfile: %s: read back format 0x%x, %d channel(s), %lld frames (wrote %d)\n", codec->name,
                (unsigned) read_info.format, read_info.channels, (long long) frames_read, FRAMES);
        return 1;
    }
    if ((codec->format & SF_FORMAT_TYPEMASK) == SF_FORMAT_FLAC) {
        /* Lossless: each sample must equal what was written, up to 16-bit quantisation. libsndfile scales by 32767
         * when it writes a float and divides by 32768 when it reads one back, so allow two 16-bit steps. */
        if (frames_read != FRAMES) {
            fprintf(stderr, "libsndfile: %s: lossless read-back has %lld frames, wrote %d\n", codec->name,
                    (long long) frames_read, FRAMES);
            return 1;
        }
        for (int index = 0; index < FRAMES; ++index) {
            if (fabsf(readback[index] - samples[index]) > 2.0F / 32768.0F) {
                fprintf(stderr, "libsndfile: %s: sample %d came back %.6f, wrote %.6f\n", codec->name, index,
                        (double) readback[index], (double) samples[index]);
                return 1;
            }
        }
        printf("libsndfile: %s round-tripped (%lld frames, every sample exact)\n", codec->name, (long long) frames_read);
        return 0;
    }

    /* Lossy: measure the tone in the window (a one-bin DFT over a whole number of periods), then what is left. */
    double in_phase = 0.0;
    double quadrature = 0.0;
    double energy = 0.0;
    for (int index = 0; index < WINDOW_FRAMES; ++index) {
        const double value = (double) readback[WINDOW_START + index];
        const double angle = 2.0 * M_PI * TONE_HZ * (double) index / (double) codec->sample_rate;
        in_phase += value * cos(angle);
        quadrature += value * sin(angle);
        energy += value * value;
    }
    const double amplitude = 2.0 * sqrt(in_phase * in_phase + quadrature * quadrature) / (double) WINDOW_FRAMES;
    const double residual_power = energy / (double) WINDOW_FRAMES - amplitude * amplitude / 2.0;
    const double residual_rms = residual_power > 0.0 ? sqrt(residual_power) : 0.0;
    if (fabs(amplitude - 0.5) > 0.05 || residual_rms > 0.05) {
        fprintf(stderr, "libsndfile: %s: decoded tone amplitude %.4f (wrote 0.5), residual RMS %.4f\n", codec->name,
                amplitude, residual_rms);
        return 1;
    }
    printf("libsndfile: %s round-tripped (%lld frames; %.0f Hz tone amplitude %.4f, residual RMS %.4f)\n",
           codec->name, (long long) frames_read, TONE_HZ, amplitude, residual_rms);
    return 0;
}

int main(void) {
    static const codec_case cases[] = {
        {"FLAC", SF_FORMAT_FLAC | SF_FORMAT_PCM_16, 48000, 1},
        {"Ogg Vorbis", SF_FORMAT_OGG | SF_FORMAT_VORBIS, 48000, 0},
        {"Ogg Opus", SF_FORMAT_OGG | PROBE_FORMAT_OPUS, 48000, 0},
        {"MP3", PROBE_FORMAT_MPEG | PROBE_FORMAT_MPEG_LAYER_III, 48000, 0},
    };

    int failures = 0;
    for (size_t index = 0; index < sizeof cases / sizeof cases[0]; ++index) {
        if (!codec_available(&cases[index])) {
            if (cases[index].required) {
                fprintf(stderr, "libsndfile: %s: not supported by this libsndfile\n", cases[index].name);
                ++failures;
            } else {
                printf("libsndfile: %s: not supported by this libsndfile, skipped\n", cases[index].name);
            }
            continue;
        }
        failures += round_trip(&cases[index]);
    }
    printf("%s\n", sf_version_string());
    return failures == 0 ? 0 : 1;
}
