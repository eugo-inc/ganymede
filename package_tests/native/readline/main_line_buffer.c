/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for readline: line buffer; calls rl_initialize(), rl_replace_line(), rl_insert_text().
 */

#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <assert.h>

int main(void) {
    assert(rl_initialize() == 0);

    /* rl_line_buffer must be allocated post-init. */
    assert(rl_line_buffer != NULL);

    /* Overwrite the buffer with a known string; second arg is clear_undo flag. */
    rl_replace_line("hello", 1);
    assert(strcmp(rl_line_buffer, "hello") == 0);
    assert(rl_end == 5);

    /* Move the cursor to the end and insert " world" — exercise rl_insert_text. */
    rl_point = rl_end;
    int inserted = rl_insert_text(" world");
    assert(inserted == 6);
    assert(strcmp(rl_line_buffer, "hello world") == 0);
    assert(rl_end == 11);
    assert(rl_point == 11);

    /* Move cursor to position 5 (after "hello") and splice "X" in. */
    rl_point = 5;
    inserted = rl_insert_text("X");
    assert(inserted == 1);
    assert(strcmp(rl_line_buffer, "helloX world") == 0);
    assert(rl_end == 12);
    assert(rl_point == 6);

    /* Replace the line again — must reset cleanly. */
    rl_replace_line("", 1);
    assert(rl_end == 0);
    assert(rl_line_buffer[0] == '\0');

    printf("readline line_buffer test OK\n");
    return 0;
}
