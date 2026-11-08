/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_auth: signing config; calls aws_default_allocator(), aws_auth_library_init(), aws_byte_cursor_from_c_str(), aws_credentials_new().
 */

#include <aws/auth/auth.h>
#include <aws/auth/credentials.h>
#include <aws/auth/signing_config.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/date_time.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_auth_library_init(alloc);

    struct aws_byte_cursor access_key = aws_byte_cursor_from_c_str("AKIAIOSFODNN7EXAMPLE");
    struct aws_byte_cursor secret_key = aws_byte_cursor_from_c_str("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    struct aws_byte_cursor session_token = aws_byte_cursor_from_c_str("");
    struct aws_credentials *creds = aws_credentials_new(alloc, access_key, secret_key, session_token, UINT64_MAX);
    assert(creds != NULL);

    struct aws_signing_config_aws config;
    AWS_ZERO_STRUCT(config);
    config.config_type = AWS_SIGNING_CONFIG_AWS;
    config.algorithm = AWS_SIGNING_ALGORITHM_V4;
    config.signature_type = AWS_ST_HTTP_REQUEST_HEADERS;
    config.region = aws_byte_cursor_from_c_str("us-east-1");
    config.service = aws_byte_cursor_from_c_str("s3");
    config.signed_body_header = AWS_SBHT_X_AMZ_CONTENT_SHA256;
    config.credentials = creds;
    aws_date_time_init_now(&config.date);

    /* A complete config should validate. */
    int rc_ok = aws_validate_aws_signing_config_aws(&config);
    assert(rc_ok == AWS_OP_SUCCESS);

    /* Clearing a required field (region) MUST cause validation to fail. */
    struct aws_byte_cursor saved_region = config.region;
    config.region = aws_byte_cursor_from_c_str("");
    int rc_bad = aws_validate_aws_signing_config_aws(&config);
    assert(rc_bad != AWS_OP_SUCCESS);
    config.region = saved_region;

    /* And re-validating after restoring region should succeed again. */
    int rc_ok2 = aws_validate_aws_signing_config_aws(&config);
    assert(rc_ok2 == AWS_OP_SUCCESS);

    aws_credentials_release(creds);
    aws_auth_library_clean_up();

    printf("aws_c_auth signing_config test OK\n");
    return 0;
}
