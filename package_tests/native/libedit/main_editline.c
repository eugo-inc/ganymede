/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libedit: editline; calls el_init(), el_set(), el_get(), el_end().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <histedit.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    EditLine *el = el_init("demo-libedit-test", stdin, stdout, stderr);
    assert(el != NULL);

    /* EL_EDITOR is a get/set string knob — switch to emacs mode and read it back. */
    assert(el_set(el, EL_EDITOR, "emacs") == 0);

    const char *editor = NULL;
    assert(el_get(el, EL_EDITOR, &editor) == 0);
    assert(editor != NULL);
    assert(strcmp(editor, "emacs") == 0);

    int sentinel = 0xC0FFEE;
    assert(el_set(el, EL_CLIENTDATA, &sentinel) == 0);

    void *retrieved = NULL;
    assert(el_get(el, EL_CLIENTDATA, &retrieved) == 0);
    assert(retrieved == &sentinel);

    el_end(el);

    printf("libedit editline test OK\n");
    return 0;
}
