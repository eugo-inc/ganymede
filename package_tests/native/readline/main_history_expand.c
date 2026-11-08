/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: history expand; calls using_history(), add_history(), history_total_bytes(), history_expand().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <assert.h>

int main(void) {
    using_history();

    add_history("ls -la /tmp");
    add_history("echo hello world");
    add_history("grep needle haystack");

    /* history_total_bytes is an aux libhistory symbol; should reflect real data. */
    assert(history_total_bytes() > 0);

    /* "!1" -> the first entry; "!!" -> the previous (most recent) entry. */
    char *expanded = NULL;
    int rc = history_expand("!1", &expanded);
    assert(rc == 1);
    assert(expanded != NULL);
    assert(strcmp(expanded, "ls -la /tmp") == 0);
    free(expanded);
    expanded = NULL;

    rc = history_expand("!!", &expanded);
    assert(rc == 1);
    assert(expanded != NULL);
    assert(strcmp(expanded, "grep needle haystack") == 0);
    free(expanded);
    expanded = NULL;

    rc = history_expand("!2", &expanded);
    assert(rc == 1);
    assert(expanded != NULL);
    assert(strcmp(expanded, "echo hello world") == 0);
    free(expanded);
    expanded = NULL;

    /* A plain string with no history metachars -> rc 0, output is a strdup of input. */
    rc = history_expand("plain string with no bang", &expanded);
    assert(rc == 0);
    assert(expanded != NULL);
    assert(strcmp(expanded, "plain string with no bang") == 0);
    free(expanded);

    clear_history();
    printf("readline history_expand test OK\n");
    return 0;
}
