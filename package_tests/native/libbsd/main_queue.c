/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libbsd: queue; compiles against <bsd/sys/queue.h>.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <bsd/sys/queue.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct node {
    int value;
    TAILQ_ENTRY(node) link;
};

TAILQ_HEAD(node_head, node);

int main(void) {
    struct node_head head;
    TAILQ_INIT(&head);
    assert(TAILQ_EMPTY(&head));

    /* Push 5 nodes with values 0..4. */
    struct node *nodes[5];
    for (int i = 0; i < 5; ++i) {
        nodes[i] = (struct node *)malloc(sizeof(struct node));
        assert(nodes[i] != NULL);
        nodes[i]->value = i;
        TAILQ_INSERT_TAIL(&head, nodes[i], link);
    }
    assert(!TAILQ_EMPTY(&head));

    /* Walk: expect 0,1,2,3,4 in order. */
    int expected = 0;
    struct node *cur;
    TAILQ_FOREACH(cur, &head, link) {
        assert(cur->value == expected);
        ++expected;
    }
    assert(expected == 5);

    /* Remove the middle node (value 2). */
    TAILQ_REMOVE(&head, nodes[2], link);
    free(nodes[2]);

    /* Walk again: expect 0,1,3,4. */
    int seen[5] = {0, 0, 0, 0, 0};
    int count = 0;
    TAILQ_FOREACH(cur, &head, link) {
        seen[cur->value] = 1;
        ++count;
    }
    assert(count == 4);
    assert(seen[0] == 1);
    assert(seen[1] == 1);
    assert(seen[2] == 0);
    assert(seen[3] == 1);
    assert(seen[4] == 1);

    /* Drain the rest. */
    for (int i = 0; i < 5; ++i) {
        if (i == 2) continue;
        TAILQ_REMOVE(&head, nodes[i], link);
        free(nodes[i]);
    }
    assert(TAILQ_EMPTY(&head));

    printf("libbsd queue test OK\n");
    return 0;
}
