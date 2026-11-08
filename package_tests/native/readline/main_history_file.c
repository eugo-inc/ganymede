/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: history file; calls history_list(), using_history(), clear_history(), add_history().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <assert.h>

static int count_history_list(void) {
    HIST_ENTRY **lst = history_list();
    if (lst == NULL) return 0;
    int n = 0;
    while (lst[n] != NULL) n++;
    return n;
}

int main(void) {
    using_history();
    clear_history();

    /* Three known entries, in insertion order. */
    add_history("alpha one");
    add_history("beta two");
    add_history("gamma three");
    assert(count_history_list() == 3);

    char path[64];
    snprintf(path, sizeof(path), "/tmp/demo_rl_hist_%d.txt", (int)getpid());

    assert(write_history(path) == 0);

    /* Wipe in-memory state, then read back from disk. */
    clear_history();
    assert(count_history_list() == 0);

    assert(read_history(path) == 0);
    assert(count_history_list() == 3);

    HIST_ENTRY *e0 = history_get(history_base + 0);
    HIST_ENTRY *e2 = history_get(history_base + 2);
    assert(e0 != NULL && e0->line != NULL);
    assert(e2 != NULL && e2->line != NULL);
    assert(strcmp(e0->line, "alpha one") == 0);
    assert(strcmp(e2->line, "gamma three") == 0);

    /* Truncate the file to its last 1 line, clear, re-read, expect 1 entry. */
    assert(history_truncate_file(path, 1) == 0);
    clear_history();
    assert(read_history(path) == 0);
    assert(count_history_list() == 1);

    HIST_ENTRY *tail = history_get(history_base + 0);
    assert(tail != NULL && tail->line != NULL);
    assert(strcmp(tail->line, "gamma three") == 0);

    clear_history();
    unlink(path);

    printf("readline history_file test OK\n");
    return 0;
}
