/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libacl: core API; calls acl_from_text(), acl_valid(), acl_to_text(), acl_free().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <sys/acl.h>
#include <acl/libacl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    const char *text = "u::rw-,g::r--,o::r--";
    acl_t acl = acl_from_text(text);
    assert(acl != (acl_t)NULL);

    /* acl_valid() (POSIX.1e, <sys/acl.h>) validates the ACL. */
    assert(acl_valid(acl) == 0);

    ssize_t len = 0;
    char *roundtrip = acl_to_text(acl, &len);
    assert(roundtrip != NULL);
    assert(len > 0);
    assert(strstr(roundtrip, "user::rw-") != NULL);
    assert(strstr(roundtrip, "group::r--") != NULL);
    assert(strstr(roundtrip, "other::r--") != NULL);

    acl_free(roundtrip);
    acl_free(acl);

    printf("libacl test OK\n");
    return 0;
}
