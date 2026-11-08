/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libacl: ext; calls acl_from_text(), acl_cmp(), acl_check(), acl_to_any_text().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <sys/acl.h>
#include <acl/libacl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    acl_t a1 = acl_from_text("u::rw-,g::r--,o::r--");
    acl_t a2 = acl_from_text("u::rw-,g::r--,o::r--");
    acl_t a3 = acl_from_text("u::rwx,g::r-x,o::r-x");
    assert(a1 != (acl_t)NULL);
    assert(a2 != (acl_t)NULL);
    assert(a3 != (acl_t)NULL);

    /* --- acl_cmp --- */
    /* a1 and a2 are structurally identical: cmp returns 0. */
    assert(acl_cmp(a1, a2) == 0);
    /* a1 and a3 differ in permset values: cmp returns 1 (not -1). */
    assert(acl_cmp(a1, a3) == 1);

    /* --- acl_check --- */
    /* a1 is a well-formed minimal ACL: returns 0, doesn't touch 'last'. */
    int last = -42;
    int check_rc = acl_check(a1, &last);
    assert(check_rc == 0);

    /* --- acl_to_any_text --- */
    /* Render a1 with a "test:" prefix and ',' separator (so output is single-line). */
    char *text = acl_to_any_text(a1, "test:", ',', TEXT_ABBREVIATE);
    assert(text != NULL);
    assert(strstr(text, "test:u::") != NULL);
    assert(strstr(text, "test:g::") != NULL);
    assert(strstr(text, "test:o::") != NULL);
    /* Separator ',' means newline-free output. */
    assert(strchr(text, '\n') == NULL);

    acl_free(text);
    acl_free(a3);
    acl_free(a2);
    acl_free(a1);
    printf("libacl ext test OK\n");
    return 0;
}
