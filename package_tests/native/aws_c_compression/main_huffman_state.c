/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_compression: huffman state; calls aws_default_allocator(), aws_compression_library_init(), aws_huffman_encoder_init(), aws_huffman_encoder_reset().
 */

#include <aws/compression/compression.h>
#include <aws/compression/huffman.h>
#include <aws/common/allocator.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static struct aws_huffman_code s_stub_encode(uint8_t symbol, void *userdata) {
    (void)symbol;
    (void)userdata;
    struct aws_huffman_code code = {0, 0};
    return code;
}

static uint8_t s_stub_decode(uint32_t bits, uint8_t *symbol, void *userdata) {
    (void)bits;
    (void)symbol;
    (void)userdata;
    return 0;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_compression_library_init(alloc);

    struct aws_huffman_symbol_coder coder = {
        .encode = s_stub_encode,
        .decode = s_stub_decode,
        .userdata = NULL,
    };

    struct aws_huffman_encoder encoder;
    aws_huffman_encoder_init(&encoder, &coder);
    assert(encoder.coder == &coder);
    assert(encoder.eos_padding == UINT8_MAX);

    encoder.overflow_bits.pattern = 0xDEADBEEF;
    encoder.overflow_bits.num_bits = 17;
    aws_huffman_encoder_reset(&encoder);
    assert(encoder.coder == &coder);                /* reset must NOT detach the coder */
    assert(encoder.eos_padding == UINT8_MAX);       /* Param is preserved across reset */
    assert(encoder.overflow_bits.pattern == 0);     /* State field was cleared */
    assert(encoder.overflow_bits.num_bits == 0);

    /* Decoder: init → toggle allow_growth and write to working_bits → reset → verify. */
    struct aws_huffman_decoder decoder;
    aws_huffman_decoder_init(&decoder, &coder);
    assert(decoder.coder == &coder);

    aws_huffman_decoder_allow_growth(&decoder, true);
    assert(decoder.allow_growth == true);

    decoder.working_bits = 0xDEADBEEFDEADBEEFULL;
    decoder.num_bits = 17;

    aws_huffman_decoder_reset(&decoder);
    assert(decoder.coder == &coder);     /* reset must NOT detach the coder */
    assert(decoder.working_bits == 0);   /* working state was cleared */
    assert(decoder.num_bits == 0);

    aws_compression_library_clean_up();

    printf("aws_c_compression huffman_state OK: encoder+decoder reset cleared working state\n");
    return 0;
}
