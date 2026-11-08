/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_auth: core API; calls aws_default_allocator(), aws_auth_library_init(), aws_byte_cursor_from_c_str(), aws_credentials_new().
 */

#include <aws/auth/auth.h>
#include <aws/auth/credentials.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_auth_library_init(alloc);

    struct aws_byte_cursor access_key = aws_byte_cursor_from_c_str("AKIAIOSFODNN7EXAMPLE");
    struct aws_byte_cursor secret_key = aws_byte_cursor_from_c_str("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    struct aws_byte_cursor session_token = aws_byte_cursor_from_c_str("");

    struct aws_credentials *creds = aws_credentials_new(alloc, access_key, secret_key, session_token, UINT64_MAX);
    assert(creds != NULL);

    struct aws_byte_cursor out_access = aws_credentials_get_access_key_id(creds);
    assert(out_access.len == access_key.len);
    assert(memcmp(out_access.ptr, access_key.ptr, access_key.len) == 0);

    struct aws_byte_cursor out_secret = aws_credentials_get_secret_access_key(creds);
    assert(out_secret.len == secret_key.len);
    assert(memcmp(out_secret.ptr, secret_key.ptr, secret_key.len) == 0);

    assert(aws_credentials_get_expiration_timepoint_seconds(creds) == UINT64_MAX);

    aws_credentials_release(creds);
    aws_auth_library_clean_up();

    printf("aws_c_auth test OK\n");
    return 0;
}
