/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_s3: platforms list; calls aws_default_allocator(), aws_s3_library_init(), aws_s3_get_platforms_with_recommended_config(), aws_array_list_length().
 */

#include <aws/s3/s3.h>
#include <aws/common/allocator.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_s3_library_init(alloc);

    struct aws_array_list recommended = aws_s3_get_platforms_with_recommended_config();
    size_t count = aws_array_list_length(&recommended);
    assert(count > 0);

    for (size_t i = 0; i < count; ++i) {
        struct aws_byte_cursor cursor;
        int rc = aws_array_list_get_at(&recommended, &cursor, i);
        assert(rc == AWS_OP_SUCCESS);
        assert(cursor.ptr != NULL);
        assert(cursor.len > 0);
    }

    aws_array_list_clean_up(&recommended);
    aws_s3_library_clean_up();

    printf("aws_c_s3 platforms_list test OK: %zu recommended instance types\n", count);
    return 0;
}
