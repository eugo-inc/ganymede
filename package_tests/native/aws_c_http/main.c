/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_http: core API; calls aws_default_allocator(), aws_http_library_init(), aws_http_message_new_request(), aws_byte_cursor_from_c_str().
 */

#include <aws/http/http.h>
#include <aws/http/request_response.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_http_library_init(alloc);

    struct aws_http_message *req = aws_http_message_new_request(alloc);
    assert(req != NULL);

    struct aws_byte_cursor method = aws_byte_cursor_from_c_str("GET");
    assert(aws_http_message_set_request_method(req, method) == AWS_OP_SUCCESS);

    struct aws_byte_cursor path = aws_byte_cursor_from_c_str("/healthz");
    assert(aws_http_message_set_request_path(req, path) == AWS_OP_SUCCESS);

    struct aws_byte_cursor out_method;
    assert(aws_http_message_get_request_method(req, &out_method) == AWS_OP_SUCCESS);
    assert(out_method.len == 3);
    assert(memcmp(out_method.ptr, "GET", 3) == 0);

    struct aws_byte_cursor out_path;
    assert(aws_http_message_get_request_path(req, &out_path) == AWS_OP_SUCCESS);
    assert(out_path.len == 8);
    assert(memcmp(out_path.ptr, "/healthz", 8) == 0);

    aws_http_message_release(req);
    aws_http_library_clean_up();

    printf("aws_c_http test OK\n");
    return 0;
}
