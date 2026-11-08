/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libacl: build; calls acl_create_entry(), acl_set_tag_type(), acl_get_permset(), acl_clear_perms().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <sys/acl.h>
#include <acl/libacl.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static void add_entry(acl_t *acl, acl_tag_t tag, int read, int write, int exec) {
    acl_entry_t entry;
    assert(acl_create_entry(acl, &entry) == 0);
    assert(acl_set_tag_type(entry, tag) == 0);

    acl_permset_t permset;
    assert(acl_get_permset(entry, &permset) == 0);
    assert(acl_clear_perms(permset) == 0);
    if (read)  assert(acl_add_perm(permset, ACL_READ) == 0);
    if (write) assert(acl_add_perm(permset, ACL_WRITE) == 0);
    if (exec)  assert(acl_add_perm(permset, ACL_EXECUTE) == 0);
    /* Permset is referenced by the entry — no separate set step needed for
     * the permset object returned by acl_get_permset on a fresh entry. */
}

int main(void) {
    /* Hint: 3 entries — libacl may grow as needed. */
    acl_t acl = acl_init(3);
    assert(acl != (acl_t)NULL);

    add_entry(&acl, ACL_USER_OBJ,  1, 1, 0);  /* rw- */
    add_entry(&acl, ACL_GROUP_OBJ, 1, 0, 1);  /* r-x */
    add_entry(&acl, ACL_OTHER,     1, 0, 0);  /* r-- */

    /* Must validate cleanly: 3 required entries, no named users/groups,
     * no mask required. */
    assert(acl_valid(acl) == 0);

    ssize_t len = 0;
    char *text = acl_to_text(acl, &len);
    assert(text != NULL);
    assert(len > 0);

    assert(strstr(text, "user::rw-")  != NULL);
    assert(strstr(text, "group::r-x") != NULL);
    assert(strstr(text, "other::r--") != NULL);

    acl_free(text);
    acl_free(acl);
    printf("libacl build test OK\n");
    return 0;
}
