/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_sdkutils: arn; calls aws_default_allocator(), aws_sdkutils_library_init(), aws_byte_cursor_from_c_str(), aws_resource_name_init_from_cur().
 */

#include <aws/sdkutils/sdkutils.h>
#include <aws/sdkutils/resource_name.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int cursor_eq_cstr(const struct aws_byte_cursor *c, const char *s) {
    size_t n = strlen(s);
    return c->len == n && memcmp(c->ptr, s, n) == 0;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_sdkutils_library_init(alloc);

    /* --- Positive: known-good CloudFormation ARN parses into 5 parts. --- */
    const char *arn_str = "arn:aws:cloudformation:us-east-1:1234567890:stack/FooBar";
    struct aws_byte_cursor input = aws_byte_cursor_from_c_str(arn_str);
    struct aws_resource_name arn;
    memset(&arn, 0, sizeof(arn));
    int rc = aws_resource_name_init_from_cur(&arn, &input);
    assert(rc == AWS_OP_SUCCESS);
    assert(cursor_eq_cstr(&arn.partition, "aws"));
    assert(cursor_eq_cstr(&arn.service, "cloudformation"));
    assert(cursor_eq_cstr(&arn.region, "us-east-1"));
    assert(cursor_eq_cstr(&arn.account_id, "1234567890"));
    assert(cursor_eq_cstr(&arn.resource_id, "stack/FooBar"));

    size_t needed = 0;
    rc = aws_resource_name_length(&arn, &needed);
    assert(rc == AWS_OP_SUCCESS);
    assert(needed == strlen(arn_str));

    struct aws_byte_buf out;
    rc = aws_byte_buf_init(&out, alloc, needed);
    assert(rc == AWS_OP_SUCCESS);
    rc = aws_byte_buf_append_resource_name(&out, &arn);
    assert(rc == AWS_OP_SUCCESS);
    assert(out.len == strlen(arn_str));
    assert(memcmp(out.buffer, arn_str, out.len) == 0);
    aws_byte_buf_clean_up(&out);

    /* --- Negative: missing resource_id is rejected. --- */
    struct aws_byte_cursor bad = aws_byte_cursor_from_c_str("arn:aws-us-gov:iam::123456789012");
    struct aws_resource_name bad_arn;
    memset(&bad_arn, 0, sizeof(bad_arn));
    rc = aws_resource_name_init_from_cur(&bad_arn, &bad);
    assert(rc != AWS_OP_SUCCESS);
    assert(aws_last_error() == AWS_ERROR_MALFORMED_INPUT_STRING);

    aws_sdkutils_library_clean_up();

    printf("aws_c_sdkutils ARN test OK: parsed + round-tripped %s\n", arn_str);
    return 0;
}
