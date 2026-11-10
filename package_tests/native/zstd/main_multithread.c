/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for zstd: multithread; calls ZSTD_cParam_getBounds(), ZSTD_isError(), ZSTD_createCCtx(), ZSTD_CCtx_setParameter().
 */

#include <zstd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE ((size_t)8 << 20)
#define JOB_SIZE (1 << 20)

static int fail(const char *what) {
    fprintf(stderr, "ZSTD-MULTITHREAD-CHECK: FAIL: %s\n", what);
    return 1;
}

int main(void) {
    ZSTD_bounds const bounds = ZSTD_cParam_getBounds(ZSTD_c_nbWorkers);
    if (ZSTD_isError(bounds.error)) {
        return fail("ZSTD_cParam_getBounds(ZSTD_c_nbWorkers) returned an error");
    }
    if (bounds.upperBound <= 0) {
        fprintf(stderr, "ZSTD-MULTITHREAD-CHECK: FAIL: nbWorkers upper bound is %d: libzstd was built without "
                        "ZSTD_MULTITHREAD (ZSTD_MULTITHREAD_SUPPORT=ON did not reach the library)\n",
                bounds.upperBound);
        return 1;
    }

    ZSTD_CCtx *cctx = ZSTD_createCCtx();
    if (cctx == NULL) {
        return fail("ZSTD_createCCtx returned NULL");
    }
    size_t rc = ZSTD_CCtx_setParameter(cctx, ZSTD_c_nbWorkers, 2);
    if (ZSTD_isError(rc)) {
        fprintf(stderr, "ZSTD-MULTITHREAD-CHECK: FAIL: setting nbWorkers=2 was refused: %s\n", ZSTD_getErrorName(rc));
        return 1;
    }
    rc = ZSTD_CCtx_setParameter(cctx, ZSTD_c_jobSize, JOB_SIZE);
    if (ZSTD_isError(rc)) {
        fprintf(stderr, "ZSTD-MULTITHREAD-CHECK: FAIL: setting jobSize was refused: %s\n", ZSTD_getErrorName(rc));
        return 1;
    }

    /* Compressible but not trivial: a repeating sentence with a running counter mixed in. */
    unsigned char *input = malloc(INPUT_SIZE);
    if (input == NULL) {
        return fail("malloc(input) failed");
    }
    for (size_t i = 0; i < INPUT_SIZE; i++) {
        input[i] = (unsigned char)("demo multithreaded zstd probe "[i % 30] ^ ((i >> 12) & 0x7));
    }
    size_t const cap = ZSTD_compressBound(INPUT_SIZE);
    unsigned char *compressed = malloc(cap);
    unsigned char *output = malloc(INPUT_SIZE);
    if (compressed == NULL || output == NULL) {
        return fail("malloc(buffers) failed");
    }
    size_t const csize = ZSTD_compress2(cctx, compressed, cap, input, INPUT_SIZE);
    if (ZSTD_isError(csize)) {
        fprintf(stderr, "ZSTD-MULTITHREAD-CHECK: FAIL: ZSTD_compress2 with 2 workers: %s\n", ZSTD_getErrorName(csize));
        return 1;
    }
    size_t const dsize = ZSTD_decompress(output, INPUT_SIZE, compressed, csize);
    if (ZSTD_isError(dsize) || dsize != INPUT_SIZE || memcmp(output, input, INPUT_SIZE) != 0) {
        return fail("the 2-worker frame does not decompress to the original 8 MiB");
    }

    printf("ZSTD-MULTITHREAD-CHECK: PASS (nbWorkers upper bound %d, 8 MiB -> %zu bytes with 2 workers)\n",
           bounds.upperBound, csize);
    free(output);
    free(compressed);
    free(input);
    ZSTD_freeCCtx(cctx);
    return 0;
}
