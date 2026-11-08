/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: completion; calls rl_initialize(), rl_completion_matches().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <assert.h>

static const char *VOCAB[] = { "foo", "food", "bar", NULL };

static char *vocab_generator(const char *text, int state) {
    static int idx;
    static size_t len;
    if (state == 0) {
        idx = 0;
        len = strlen(text);
    }
    while (VOCAB[idx] != NULL) {
        const char *candidate = VOCAB[idx++];
        if (strncmp(candidate, text, len) == 0) {
            /* readline takes ownership of the returned strdup'd string. */
            return strdup(candidate);
        }
    }
    return NULL;
}

int main(void) {
    assert(rl_initialize() == 0);

    assert(rl_basic_word_break_characters != NULL);
    assert(strlen(rl_basic_word_break_characters) > 0);

    char **matches = rl_completion_matches("fo", vocab_generator);
    assert(matches != NULL);

    /* Per readline's contract, matches[0] is the longest common prefix of all
     * found matches ("foo" here), followed by each match, then a trailing NULL. */
    assert(matches[0] != NULL);
    assert(strcmp(matches[0], "foo") == 0);
    assert(matches[1] != NULL);
    assert(matches[2] != NULL);
    assert(matches[3] == NULL);

    int saw_foo = 0, saw_food = 0;
    for (int i = 1; i <= 2; i++) {
        if (strcmp(matches[i], "foo") == 0) saw_foo = 1;
        else if (strcmp(matches[i], "food") == 0) saw_food = 1;
    }
    assert(saw_foo && saw_food);

    /* Caller owns the array + each entry. */
    for (int i = 0; matches[i] != NULL; i++) free(matches[i]);
    free(matches);

    printf("readline completion test OK\n");
    return 0;
}
