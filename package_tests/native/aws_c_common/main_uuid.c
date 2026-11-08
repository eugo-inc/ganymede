/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_common: uuid; calls aws_default_allocator(), aws_common_library_init(), aws_uuid_init(), aws_byte_buf_from_empty_array().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/uuid.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    struct aws_uuid u1;
    int rc = aws_uuid_init(&u1);
    assert(rc == AWS_OP_SUCCESS);

    /* AWS_UUID_STR_LEN is 37 (36 chars + NUL). Render into a fixed buffer
     * via aws_byte_buf and confirm the canonical form. */
    char str_storage[AWS_UUID_STR_LEN] = {0};
    struct aws_byte_buf buf = aws_byte_buf_from_empty_array(str_storage, sizeof(str_storage));
    rc = aws_uuid_to_str(&u1, &buf);
    assert(rc == AWS_OP_SUCCESS);
    assert(buf.len == 36);
    /* Hyphen positions per RFC 4122 canonical form. */
    assert(str_storage[8] == '-');
    assert(str_storage[13] == '-');
    assert(str_storage[18] == '-');
    assert(str_storage[23] == '-');

    /* Roundtrip: parse the rendered string back and compare bytes. */
    struct aws_uuid u1_parsed;
    struct aws_byte_cursor str_cursor = aws_byte_cursor_from_array(str_storage, 36);
    rc = aws_uuid_init_from_str(&u1_parsed, &str_cursor);
    assert(rc == AWS_OP_SUCCESS);
    assert(memcmp(u1.uuid_data, u1_parsed.uuid_data, sizeof(u1.uuid_data)) == 0);

    /* Two consecutive inits must differ (catches a stuck PRNG). */
    struct aws_uuid u2;
    rc = aws_uuid_init(&u2);
    assert(rc == AWS_OP_SUCCESS);
    assert(memcmp(u1.uuid_data, u2.uuid_data, sizeof(u1.uuid_data)) != 0);

    /* Malformed input must be rejected. */
    struct aws_uuid bogus;
    struct aws_byte_cursor bad = aws_byte_cursor_from_c_str("not-a-uuid");
    rc = aws_uuid_init_from_str(&bogus, &bad);
    assert(rc == AWS_OP_ERR);

    aws_common_library_clean_up();
    printf("aws_c_common uuid test OK\n");
    return 0;
}
