/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_io: tls conn options; calls aws_default_allocator(), aws_io_library_init(), aws_tls_ctx_options_init_default_client(), aws_tls_client_ctx_new().
 */

#include <aws/io/io.h>
#include <aws/io/tls_channel_handler.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    struct aws_tls_ctx_options tls_options;
    aws_tls_ctx_options_init_default_client(&tls_options, alloc);

    struct aws_tls_ctx *tls_ctx = aws_tls_client_ctx_new(alloc, &tls_options);
    assert(tls_ctx != NULL);

    struct aws_tls_connection_options conn_options;
    aws_tls_connection_options_init_from_ctx(&conn_options, tls_ctx);

    const char host[] = "example.com";
    struct aws_byte_cursor host_cursor = {
        .ptr = (uint8_t *)host,
        .len = sizeof(host) - 1,
    };
    int rc = aws_tls_connection_options_set_server_name(&conn_options, alloc, &host_cursor);
    assert(rc == AWS_OP_SUCCESS);

    /* ALPN: a typical HTTP/2 + HTTP/1.1 negotiation list. The parser
     * tokenises on ';' and stores up to 4 protocols; a bad parse leaves
     * the alpn list NULL and the call would still report success — so
     * this catches construction but not parse correctness. The non-NULL
     * post-condition is verified below. */
    rc = aws_tls_connection_options_set_alpn_list(&conn_options, alloc, "h2;http/1.1");
    assert(rc == AWS_OP_SUCCESS);
    assert(conn_options.alpn_list != NULL);
    assert(conn_options.server_name != NULL);

    aws_tls_connection_options_clean_up(&conn_options);
    aws_tls_ctx_release(tls_ctx);
    aws_tls_ctx_options_clean_up(&tls_options);

    aws_io_library_clean_up();

    printf("aws_c_io tls_conn_options test OK\n");
    return 0;
}
