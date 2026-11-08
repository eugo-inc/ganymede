/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for readline: tilde; calls tilde_expand(), tilde_expand_word().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/tilde.h>
#include <assert.h>

int main(void) {
    char *plain = tilde_expand("/no/tilde/here.txt");
    assert(plain != NULL);
    assert(strcmp(plain, "/no/tilde/here.txt") == 0);
    free(plain);

    char *home = tilde_expand("~");
    assert(home != NULL);
    assert(strlen(home) > 0);
    assert(home[0] == '/');
    free(home);

    char *word_home = tilde_expand_word("~");
    assert(word_home != NULL);
    assert(strlen(word_home) > 0);
    assert(word_home[0] == '/');
    free(word_home);

    (void)&tilde_additional_prefixes;

    printf("readline tilde test OK\n");
    return 0;
}
