/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_common: linked list; calls aws_default_allocator(), aws_common_library_init(), aws_linked_list_init(), aws_linked_list_empty().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/linked_list.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

struct item {
    int value;
    struct aws_linked_list_node node;
};

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    struct aws_linked_list list;
    aws_linked_list_init(&list);
    assert(aws_linked_list_empty(&list));

    struct item a = {.value = 10};
    struct item b = {.value = 20};
    struct item c = {.value = 30};

    aws_linked_list_push_back(&list, &a.node);
    aws_linked_list_push_back(&list, &b.node);
    aws_linked_list_push_back(&list, &c.node);

    assert(!aws_linked_list_empty(&list));

    /* front/back accessors point at the expected end nodes. */
    struct aws_linked_list_node *front = aws_linked_list_front(&list);
    struct aws_linked_list_node *back = aws_linked_list_back(&list);
    struct item *front_item = AWS_CONTAINER_OF(front, struct item, node);
    struct item *back_item = AWS_CONTAINER_OF(back, struct item, node);
    assert(front_item->value == 10);
    assert(back_item->value == 30);

    /* Pop in FIFO order and verify values. */
    int expected[] = {10, 20, 30};
    for (size_t i = 0; i < 3; ++i) {
        assert(!aws_linked_list_empty(&list));
        struct aws_linked_list_node *n = aws_linked_list_pop_front(&list);
        struct item *it = AWS_CONTAINER_OF(n, struct item, node);
        assert(it->value == expected[i]);
    }
    assert(aws_linked_list_empty(&list));

    aws_common_library_clean_up();
    printf("aws_c_common linked_list test OK\n");
    return 0;
}
