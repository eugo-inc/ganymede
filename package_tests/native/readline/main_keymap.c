/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: keymap; calls rl_initialize(), rl_get_keymap(), rl_make_bare_keymap(), rl_set_keymap().
 */

#include <stdio.h>
#include <readline/readline.h>
#include <assert.h>

int main(void) {
    assert(rl_initialize() == 0);

    Keymap original = rl_get_keymap();
    assert(original != NULL);

    Keymap fresh = rl_make_bare_keymap();
    assert(fresh != NULL);
    rl_set_keymap(fresh);
    assert(rl_get_keymap() == fresh);

    rl_command_func_t *beg_of_line = rl_named_function("beginning-of-line");
    assert(beg_of_line != NULL);

    /* Ctrl-A == 0x01. rl_bind_key returns 0 on success. */
    assert(rl_bind_key(0x01, beg_of_line) == 0);

    int type = 0;
    rl_command_func_t *found = rl_function_of_keyseq("\001", fresh, &type);
    assert(found == beg_of_line);

    /* Restore the original keymap and free the throwaway. */
    rl_set_keymap(original);
    rl_discard_keymap(fresh);

    printf("readline keymap test OK\n");
    return 0;
}
