/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zstd: zdict; calls ZSTD_getErrorName(), ZDICT_trainFromBuffer(), ZDICT_isError(), ZDICT_getErrorName().
 */

#include <zdict.h>
#include <zstd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_COUNT 2000
#define DICT_CAPACITY 4096

static const char *const kNames[] = {"alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel"};
static const char *const kStates[] = {"queued", "running", "succeeded", "failed"};

static int fail(const char *what, size_t code) {
    fprintf(stderr, "ZSTD-ZDICT-CHECK: FAIL: %s%s%s\n", what, code ? ": " : "",
            code ? ZSTD_getErrorName(code) : "");
    return 1;
}

static size_t make_record(char *out, size_t cap, unsigned i) {
    int n = snprintf(out, cap,
                     "{\"job_id\": %u, \"owner\": \"%s\", \"state\": \"%s\", \"cpu_demand\": %u, "
                     "\"memory_gib\": %u, \"platform\": \"aarch64/aws_graviton5\"}",
                     100000u + i * 7919u % 99991u, kNames[i % 8], kStates[(i / 3) % 4], 1u + i % 64u,
                     4u + (i * 13u) % 120u);
    return n > 0 ? (size_t)n : 0;
}

int main(void) {
    char *samples = malloc((size_t)SAMPLE_COUNT * 256);
    size_t *sizes = malloc(sizeof(size_t) * SAMPLE_COUNT);
    void *dict = malloc(DICT_CAPACITY);
    if (samples == NULL || sizes == NULL || dict == NULL) {
        return fail("malloc failed", 0);
    }
    size_t total = 0;
    for (unsigned i = 0; i < SAMPLE_COUNT; i++) {
        sizes[i] = make_record(samples + total, 256, i);
        total += sizes[i];
    }

    size_t const dict_size = ZDICT_trainFromBuffer(dict, DICT_CAPACITY, samples, sizes, SAMPLE_COUNT);
    if (ZDICT_isError(dict_size)) {
        fprintf(stderr, "ZSTD-ZDICT-CHECK: FAIL: ZDICT_trainFromBuffer: %s\n", ZDICT_getErrorName(dict_size));
        return 1;
    }
    unsigned const dict_id = ZDICT_getDictID(dict, dict_size);
    if (dict_id == 0) {
        return fail("the trained dictionary has dictID 0 (not a zstd-format dictionary)", 0);
    }

    char record[256];
    size_t const record_size = make_record(record, sizeof record, 4242);
    size_t const cap = ZSTD_compressBound(record_size);
    char *with_dict = malloc(cap);
    char *without_dict = malloc(cap);
    char *round_trip = malloc(record_size);
    ZSTD_CDict *cdict = ZSTD_createCDict(dict, dict_size, 3);
    ZSTD_DDict *ddict = ZSTD_createDDict(dict, dict_size);
    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    ZSTD_DCtx *dctx = ZSTD_createDCtx();
    if (!with_dict || !without_dict || !round_trip || !cdict || !ddict || !cctx || !dctx) {
        return fail("allocating the dictionary contexts failed", 0);
    }

    size_t const csize = ZSTD_compress_usingCDict(cctx, with_dict, cap, record, record_size, cdict);
    if (ZSTD_isError(csize)) {
        return fail("ZSTD_compress_usingCDict", csize);
    }
    unsigned const frame_id = ZSTD_getDictID_fromFrame(with_dict, csize);
    if (frame_id != dict_id) {
        fprintf(stderr, "ZSTD-ZDICT-CHECK: FAIL: the frame names dictID %u, the dictionary is %u\n", frame_id, dict_id);
        return 1;
    }
    size_t const dsize = ZSTD_decompress_usingDDict(dctx, round_trip, record_size, with_dict, csize, ddict);
    if (ZSTD_isError(dsize) || dsize != record_size || memcmp(round_trip, record, record_size) != 0) {
        return fail("the dictionary-compressed record does not decompress to the original", 0);
    }
    size_t const plain = ZSTD_compress(without_dict, cap, record, record_size, 3);
    if (ZSTD_isError(plain)) {
        return fail("ZSTD_compress", plain);
    }
    if (csize >= plain) {
        fprintf(stderr, "ZSTD-ZDICT-CHECK: FAIL: with the trained dictionary the record is %zu bytes, without it "
                        "%zu: the dictionary does not help\n", csize, plain);
        return 1;
    }

    printf("ZSTD-ZDICT-CHECK: PASS (dictID %u, %zu-byte dictionary; record %zu -> %zu bytes, %zu without)\n",
           dict_id, dict_size, record_size, csize, plain);
    ZSTD_freeDCtx(dctx);
    ZSTD_freeCCtx(cctx);
    ZSTD_freeDDict(ddict);
    ZSTD_freeCDict(cdict);
    free(round_trip);
    free(without_dict);
    free(with_dict);
    free(dict);
    free(sizes);
    free(samples);
    return 0;
}
