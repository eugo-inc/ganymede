/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_http: response; calls aws_default_allocator(), aws_http_library_init(), aws_http_message_new_response(), aws_http_message_get_response_status().
 */

#include <aws/http/http.h>
#include <aws/http/request_response.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_http_library_init(alloc);

    struct aws_http_message *resp = aws_http_message_new_response(alloc);
    assert(resp != NULL);

    /* Unset status MUST fail per request_response.h doc comment. */
    int status = 0;
    int rc = aws_http_message_get_response_status(resp, &status);
    assert(rc == AWS_OP_ERR);

    /* Set 200, read back. */
    assert(aws_http_message_set_response_status(resp, 200) == AWS_OP_SUCCESS);
    assert(aws_http_message_get_response_status(resp, &status) == AWS_OP_SUCCESS);
    assert(status == 200);

    /* Overwrite with 404, read back. */
    assert(aws_http_message_set_response_status(resp, 404) == AWS_OP_SUCCESS);
    assert(aws_http_message_get_response_status(resp, &status) == AWS_OP_SUCCESS);
    assert(status == 404);

    aws_http_message_release(resp);
    aws_http_library_clean_up();

    printf("aws_c_http response test OK\n");
    return 0;
}
