/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libacl: iterate; calls acl_from_text(), acl_valid(), acl_get_entry(), acl_get_tag_type().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <sys/acl.h>
#include <acl/libacl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* u::rw-, g::r--, o::r-- — three required POSIX entries, no named users. */
    acl_t acl = acl_from_text("u::rw-,g::r--,o::r--");
    assert(acl != (acl_t)NULL);
    assert(acl_valid(acl) == 0);

    int saw_user_obj = 0;
    int saw_group_obj = 0;
    int saw_other = 0;
    int entry_count = 0;

    acl_entry_t entry;
    int rc = acl_get_entry(acl, ACL_FIRST_ENTRY, &entry);
    /* libacl: 1 = entry returned, 0 = no more, -1 = error. */
    while (rc == 1) {
        entry_count++;

        acl_tag_t tag = ACL_UNDEFINED_TAG;
        assert(acl_get_tag_type(entry, &tag) == 0);

        acl_permset_t permset;
        assert(acl_get_permset(entry, &permset) == 0);

        /* acl_get_perm returns 1 if the perm is in the set, 0 if absent. */
        int has_read = acl_get_perm(permset, ACL_READ);
        int has_write = acl_get_perm(permset, ACL_WRITE);
        int has_exec = acl_get_perm(permset, ACL_EXECUTE);
        assert(has_read >= 0 && has_write >= 0 && has_exec >= 0);

        if (tag == ACL_USER_OBJ) {
            saw_user_obj++;
            assert(has_read == 1);
            assert(has_write == 1);
            assert(has_exec == 0);
        } else if (tag == ACL_GROUP_OBJ) {
            saw_group_obj++;
            assert(has_read == 1);
            assert(has_write == 0);
            assert(has_exec == 0);
        } else if (tag == ACL_OTHER) {
            saw_other++;
            assert(has_read == 1);
            assert(has_write == 0);
            assert(has_exec == 0);
        } else {
            fprintf(stderr, "unexpected tag type: %d\n", (int)tag);
            return 2;
        }

        rc = acl_get_entry(acl, ACL_NEXT_ENTRY, &entry);
    }
    assert(rc == 0);  /* clean termination, not error. */

    assert(entry_count == 3);
    assert(saw_user_obj == 1);
    assert(saw_group_obj == 1);
    assert(saw_other == 1);

    acl_free(acl);
    printf("libacl iterate test OK\n");
    return 0;
}
