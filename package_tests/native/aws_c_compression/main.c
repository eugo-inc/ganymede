/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_compression: core API; calls aws_default_allocator(), aws_compression_library_init(), aws_error_str(), aws_huffman_encoder_init().
 */

#include <aws/compression/compression.h>
#include <aws/compression/huffman.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Stub symbol coder — never invoked; only its address is stored in the encoder/decoder. */
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

    /* The compression package registers its error table on init.  Resolving the sentinel
     * code MUST yield the canonical message string — catches a broken aws_register_error_info. */
    const char *err = aws_error_str(AWS_ERROR_COMPRESSION_UNKNOWN_SYMBOL);
    assert(err != NULL);
    assert(strstr(err, "unknown symbol") != NULL);

    /* Exercise the huffman encoder/decoder init paths with a stub coder.  These resolve
     * symbols from huffman.c — the largest object file in this library. */
    struct aws_huffman_symbol_coder coder = {
        .encode = s_stub_encode,
        .decode = s_stub_decode,
        .userdata = NULL,
    };

    struct aws_huffman_encoder encoder;
    aws_huffman_encoder_init(&encoder, &coder);
    assert(encoder.coder == &coder);

    struct aws_huffman_decoder decoder;
    aws_huffman_decoder_init(&decoder, &coder);
    assert(decoder.coder == &coder);
    aws_huffman_decoder_allow_growth(&decoder, true);
    assert(decoder.allow_growth == true);

    aws_compression_library_clean_up();

    printf("aws_c_compression test OK: error_str(UNKNOWN_SYMBOL)=\"%s\"\n", err);
    return 0;
}
