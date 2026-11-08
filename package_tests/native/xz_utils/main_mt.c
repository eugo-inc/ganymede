/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xz_utils: mt; calls lzma_cputhreads(), lzma_stream_encoder_mt(), lzma_code(), lzma_end().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t cpu_threads = lzma_cputhreads();
    (void)cpu_threads;

    unsigned char input[8192];
    for (size_t i = 0; i < sizeof(input); ++i)
        input[i] = (unsigned char)((i * 7) ^ (i >> 3));
    const size_t input_size = sizeof(input);

    lzma_mt mt;
    memset(&mt, 0, sizeof(mt));
    mt.flags = 0;
    mt.threads = 2;             /* explicit count; not "use lzma_cputhreads()" */
    mt.block_size = 0;           /* let liblzma pick */
    mt.timeout = 0;              /* no per-chunk timeout */
    mt.preset = 6;               /* default preset, like the CLI */
    mt.filters = NULL;           /* mutually exclusive with preset */
    mt.check = LZMA_CHECK_CRC64;

    lzma_stream enc = LZMA_STREAM_INIT;
    lzma_ret rc = lzma_stream_encoder_mt(&enc, &mt);
    assert(rc == LZMA_OK);

    unsigned char compressed[16384];
    enc.next_in = input;
    enc.avail_in = input_size;
    enc.next_out = compressed;
    enc.avail_out = sizeof(compressed);

    rc = lzma_code(&enc, LZMA_FINISH);
    assert(rc == LZMA_STREAM_END);
    const size_t compressed_size = sizeof(compressed) - enc.avail_out;
    assert(compressed_size > 0);
    lzma_end(&enc);

    /* Decode with the single-threaded stream decoder — the .xz payload
     * MT produces must be readable by any normal liblzma consumer. */
    lzma_stream dec = LZMA_STREAM_INIT;
    rc = lzma_stream_decoder(&dec, UINT64_MAX, 0);
    assert(rc == LZMA_OK);

    unsigned char decompressed[sizeof(input)];
    dec.next_in = compressed;
    dec.avail_in = compressed_size;
    dec.next_out = decompressed;
    dec.avail_out = sizeof(decompressed);

    rc = lzma_code(&dec, LZMA_FINISH);
    assert(rc == LZMA_STREAM_END);
    assert(sizeof(decompressed) - dec.avail_out == input_size);
    assert(memcmp(decompressed, input, input_size) == 0);
    lzma_end(&dec);

    printf("xz_utils mt-encoder test OK: %zu -> %zu bytes (%u threads, cpu=%u)\n",
           input_size, compressed_size, mt.threads, cpu_threads);
    return 0;
}
