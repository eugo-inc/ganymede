/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libraqm: refcount; calls raqm_create(), raqm_set_text_utf8(), raqm_reference(), raqm_clear_contents().
 */

#include <raqm.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    raqm_t *rq = raqm_create();
    assert(rq != NULL);

    const char *text = "abc";
    assert(raqm_set_text_utf8(rq, text, strlen(text)) == 1);

    /* raqm_reference must return the same pointer (refcount-style aliasing). */
    raqm_t *rq2 = raqm_reference(rq);
    assert(rq2 == rq);

    /* clear_contents must not free the context — it only resets state. */
    raqm_clear_contents(rq);

    const char *text2 = "xyz";
    assert(raqm_set_text_utf8(rq, text2, strlen(text2)) == 1);

    /* Two destroys for two references. If refcount is broken, this is a UAF. */
    raqm_destroy(rq2);
    raqm_destroy(rq);

    printf("libraqm refcount test OK\n");
    return 0;
}
