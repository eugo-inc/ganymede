/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_common: priority queue; calls aws_default_allocator(), aws_common_library_init(), aws_priority_queue_init_dynamic(), aws_priority_queue_size().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/priority_queue.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static int int_compare(const void *lhs, const void *rhs) {
    int a = *(const int *)lhs;
    int b = *(const int *)rhs;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    struct aws_priority_queue pq;
    int rc = aws_priority_queue_init_dynamic(
        &pq, alloc, /*default_size*/ 4, sizeof(int), int_compare);
    assert(rc == AWS_OP_SUCCESS);
    assert(aws_priority_queue_size(&pq) == 0);

    /* Push deliberately unordered. */
    int input[] = {42, 7, 19, 3, 27, 11};
    size_t n = sizeof(input) / sizeof(input[0]);
    for (size_t i = 0; i < n; ++i) {
        rc = aws_priority_queue_push(&pq, &input[i]);
        assert(rc == AWS_OP_SUCCESS);
    }
    assert(aws_priority_queue_size(&pq) == n);

    /* Top must be the min before any pop. */
    int *top_ptr = NULL;
    rc = aws_priority_queue_top(&pq, (void **)&top_ptr);
    assert(rc == AWS_OP_SUCCESS);
    assert(top_ptr != NULL);
    assert(*top_ptr == 3);

    int sorted_expected[] = {3, 7, 11, 19, 27, 42};
    for (size_t i = 0; i < n; ++i) {
        int got = -1;
        rc = aws_priority_queue_pop(&pq, &got);
        assert(rc == AWS_OP_SUCCESS);
        assert(got == sorted_expected[i]);
    }
    assert(aws_priority_queue_size(&pq) == 0);

    /* Pop on empty must fail. */
    int unused = 0;
    rc = aws_priority_queue_pop(&pq, &unused);
    assert(rc == AWS_OP_ERR);

    aws_priority_queue_clean_up(&pq);
    aws_common_library_clean_up();
    printf("aws_c_common priority_queue test OK\n");
    return 0;
}
