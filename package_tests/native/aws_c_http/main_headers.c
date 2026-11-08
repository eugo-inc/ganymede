/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_http: headers; calls aws_default_allocator(), aws_http_library_init(), aws_http_headers_new(), aws_http_headers_count().
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

    struct aws_http_headers *headers = aws_http_headers_new(alloc);
    assert(headers != NULL);
    assert(aws_http_headers_count(headers) == 0);

    struct aws_byte_cursor name = aws_byte_cursor_from_c_str("Content-Type");
    struct aws_byte_cursor value = aws_byte_cursor_from_c_str("application/json");
    assert(aws_http_headers_add(headers, name, value) == AWS_OP_SUCCESS);
    assert(aws_http_headers_count(headers) == 1);
    assert(aws_http_headers_has(headers, name) == true);

    struct aws_byte_cursor got_value;
    assert(aws_http_headers_get(headers, name, &got_value) == AWS_OP_SUCCESS);
    assert(got_value.len == 16);
    assert(memcmp(got_value.ptr, "application/json", 16) == 0);

    /* set must REPLACE not append. */
    struct aws_byte_cursor new_value = aws_byte_cursor_from_c_str("text/plain");
    assert(aws_http_headers_set(headers, name, new_value) == AWS_OP_SUCCESS);
    assert(aws_http_headers_count(headers) == 1);
    assert(aws_http_headers_get(headers, name, &got_value) == AWS_OP_SUCCESS);
    assert(got_value.len == 10);
    assert(memcmp(got_value.ptr, "text/plain", 10) == 0);

    /* erase removes it. */
    assert(aws_http_headers_erase(headers, name) == AWS_OP_SUCCESS);
    assert(aws_http_headers_count(headers) == 0);
    assert(aws_http_headers_has(headers, name) == false);

    aws_http_headers_release(headers);
    aws_http_library_clean_up();

    printf("aws_c_http headers test OK\n");
    return 0;
}
