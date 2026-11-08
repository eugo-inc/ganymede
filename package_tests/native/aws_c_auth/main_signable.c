/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_auth: signable; calls aws_default_allocator(), aws_http_library_init(), aws_auth_library_init(), aws_http_message_new_request().
 */

#include <aws/auth/auth.h>
#include <aws/auth/signable.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/http/http.h>
#include <aws/http/request_response.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_http_library_init(alloc);
    aws_auth_library_init(alloc);

    struct aws_http_message *request = aws_http_message_new_request(alloc);
    assert(request != NULL);

    struct aws_byte_cursor method = aws_byte_cursor_from_c_str("GET");
    struct aws_byte_cursor path = aws_byte_cursor_from_c_str("/index.html");
    assert(aws_http_message_set_request_method(request, method) == AWS_OP_SUCCESS);
    assert(aws_http_message_set_request_path(request, path) == AWS_OP_SUCCESS);

    struct aws_http_header host_header = {
        .name = aws_byte_cursor_from_c_str("Host"),
        .value = aws_byte_cursor_from_c_str("example.amazonaws.com"),
    };
    assert(aws_http_message_add_header(request, host_header) == AWS_OP_SUCCESS);

    struct aws_signable *signable = aws_signable_new_http_request(alloc, request);
    assert(signable != NULL);

    struct aws_byte_cursor out_method;
    AWS_ZERO_STRUCT(out_method);
    int rc_method = aws_signable_get_property(signable, g_aws_http_method_property_name, &out_method);
    assert(rc_method == AWS_OP_SUCCESS);
    assert(out_method.len == method.len);
    assert(memcmp(out_method.ptr, method.ptr, method.len) == 0);

    struct aws_byte_cursor out_uri;
    AWS_ZERO_STRUCT(out_uri);
    int rc_uri = aws_signable_get_property(signable, g_aws_http_uri_property_name, &out_uri);
    assert(rc_uri == AWS_OP_SUCCESS);
    assert(out_uri.len == path.len);
    assert(memcmp(out_uri.ptr, path.ptr, path.len) == 0);

    /* Header list — confirm the property-list accessor returns a non-empty list. */
    const struct aws_array_list *headers = NULL;
    int rc_headers = aws_signable_get_property_list(signable, g_aws_http_headers_property_list_name, &headers);
    assert(rc_headers == AWS_OP_SUCCESS);
    assert(headers != NULL);
    assert(aws_array_list_length(headers) >= 1);

    aws_signable_destroy(signable);
    aws_http_message_release(request);

    aws_auth_library_clean_up();
    aws_http_library_clean_up();

    printf("aws_c_auth signable test OK\n");
    return 0;
}
