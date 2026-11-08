/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xz_utils: index; calls lzma_index_init(), lzma_index_append(), lzma_index_block_count(), lzma_index_stream_count().
 */

#include <lzma.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    lzma_index *idx = lzma_index_init(NULL);
    assert(idx != NULL);

    /* Append three fake-but-valid block records. Each record is
     * (unpadded_size, uncompressed_size). unpadded_size must be > 0
     * and uncompressed_size has no such constraint.
     *
     * Use plausible per-block sizes: small compressed payloads with
     * larger uncompressed counterparts. */
    lzma_ret rc = lzma_index_append(idx, NULL, /*unpadded=*/128, /*uncomp=*/1024);
    assert(rc == LZMA_OK);
    rc = lzma_index_append(idx, NULL, 256, 2048);
    assert(rc == LZMA_OK);
    rc = lzma_index_append(idx, NULL, 64, 512);
    assert(rc == LZMA_OK);

    assert(lzma_index_block_count(idx) == 3);
    assert(lzma_index_stream_count(idx) == 1);

    const lzma_vli encoded_size = lzma_index_size(idx);
    assert(encoded_size > 0);

    /* Encode the index into a buffer. */
    unsigned char buf[512];
    assert(encoded_size <= sizeof(buf));
    size_t out_pos = 0;
    rc = lzma_index_buffer_encode(idx, buf, &out_pos, sizeof(buf));
    assert(rc == LZMA_OK);
    assert(out_pos == encoded_size);

    /* Decode the buffer back into a fresh Index. */
    lzma_index *idx2 = NULL;
    uint64_t memlimit = UINT64_MAX;
    size_t in_pos = 0;
    rc = lzma_index_buffer_decode(&idx2, &memlimit, NULL, buf, &in_pos, out_pos);
    assert(rc == LZMA_OK);
    assert(idx2 != NULL);
    assert(in_pos == out_pos);

    assert(lzma_index_block_count(idx2) == lzma_index_block_count(idx));
    assert(lzma_index_stream_count(idx2) == 1);
    assert(lzma_index_size(idx2) == encoded_size);

    lzma_index_end(idx, NULL);
    lzma_index_end(idx2, NULL);

    printf("xz_utils index test OK: 3 blocks, %llu-byte encoded index\n",
           (unsigned long long)encoded_size);
    return 0;
}
