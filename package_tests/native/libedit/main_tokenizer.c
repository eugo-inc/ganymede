/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libedit: tokenizer; calls tok_init(), tok_str(), tok_end().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <histedit.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    Tokenizer *t = tok_init(NULL);
    assert(t != NULL);

    const char *line = "echo \"hello world\" foo";
    int argc = -1;
    const char **argv = NULL;

    /* tok_str returns 0 on success; argc/argv are populated for the caller. */
    int rc = tok_str(t, line, &argc, &argv);
    assert(rc == 0);
    assert(argc == 3);
    assert(argv != NULL);

    assert(strcmp(argv[0], "echo") == 0);
    assert(strcmp(argv[1], "hello world") == 0);
    assert(strcmp(argv[2], "foo") == 0);

    tok_end(t);

    printf("libedit tokenizer test OK\n");
    return 0;
}
