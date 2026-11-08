/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: core API; calls rl_initialize(), using_history(), add_history(), history_get().
 */

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* readline init — returns 0 on success. Forces libreadline symbol resolution. */
    assert(rl_initialize() == 0);

    /* rl_library_version is a const char * global exported by libreadline. */
    assert(rl_library_version != NULL);
    assert(strlen(rl_library_version) > 0);

    using_history();
    add_history("ls -la");
    add_history("echo hello");

    HIST_ENTRY *entry = history_get(history_base + 1);
    assert(entry != NULL);
    assert(entry->line != NULL);
    assert(strcmp(entry->line, "echo hello") == 0);

    clear_history();

    printf("readline test OK: %s\n", rl_library_version);
    return 0;
}
