/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_io: tls; calls aws_default_allocator(), aws_io_library_init(), aws_tls_ctx_options_init_default_client(), aws_tls_is_cipher_pref_supported().
 */

#include <aws/io/io.h>
#include <aws/io/tls_channel_handler.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    struct aws_tls_ctx_options tls_options;
    aws_tls_ctx_options_init_default_client(&tls_options, alloc);

    assert(aws_tls_is_cipher_pref_supported(tls_options.cipher_pref));

    struct aws_tls_ctx *tls_ctx = aws_tls_client_ctx_new(alloc, &tls_options);
    assert(tls_ctx != NULL);

    aws_tls_ctx_release(tls_ctx);
    aws_tls_ctx_options_clean_up(&tls_options);

    aws_io_library_clean_up();

    printf("aws_c_io tls_ctx test OK\n");
    return 0;
}
