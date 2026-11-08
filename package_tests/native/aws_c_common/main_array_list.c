/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_common: array list; calls aws_default_allocator(), aws_common_library_init(), aws_array_list_init_dynamic(), aws_array_list_length().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/array_list.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    struct aws_array_list list;
    int rc = aws_array_list_init_dynamic(&list, alloc, /*initial_item_allocation*/ 2, sizeof(int));
    assert(rc == AWS_OP_SUCCESS);
    assert(aws_array_list_length(&list) == 0);

    for (int i = 0; i < 5; ++i) {
        int v = i * 10;
        rc = aws_array_list_push_back(&list, &v);
        assert(rc == AWS_OP_SUCCESS);
    }
    assert(aws_array_list_length(&list) == 5);

    /* Indexed reads — verify every element survived the grow. */
    for (size_t i = 0; i < 5; ++i) {
        int got = -1;
        rc = aws_array_list_get_at(&list, &got, i);
        assert(rc == AWS_OP_SUCCESS);
        assert(got == (int)(i * 10));
    }

    /* Out-of-bounds get must fail (returns AWS_OP_ERR, sets last_error). */
    int oob = 0;
    rc = aws_array_list_get_at(&list, &oob, 99);
    assert(rc == AWS_OP_ERR);

    aws_array_list_clean_up(&list);
    aws_common_library_clean_up();
    printf("aws_c_common array_list test OK\n");
    return 0;
}
