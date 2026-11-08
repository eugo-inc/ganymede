/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_common: hash table; calls aws_default_allocator(), aws_common_library_init(), aws_hash_table_init(), aws_hash_table_get_entry_count().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/hash_table.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_common_library_init(alloc);

    struct aws_hash_table table;
    int rc = aws_hash_table_init(
        &table, alloc, /*size*/ 8,
        aws_hash_c_string, aws_hash_callback_c_str_eq,
        /*destroy_key_fn*/ NULL, /*destroy_value_fn*/ NULL);
    assert(rc == AWS_OP_SUCCESS);
    assert(aws_hash_table_get_entry_count(&table) == 0);

    int was_created = 0;
    rc = aws_hash_table_put(&table, "alpha", (void *)"one", &was_created);
    assert(rc == AWS_OP_SUCCESS);
    assert(was_created == 1);
    rc = aws_hash_table_put(&table, "beta", (void *)"two", &was_created);
    assert(rc == AWS_OP_SUCCESS);
    assert(was_created == 1);
    assert(aws_hash_table_get_entry_count(&table) == 2);

    /* Lookup hits. */
    struct aws_hash_element *elem = NULL;
    rc = aws_hash_table_find(&table, "alpha", &elem);
    assert(rc == AWS_OP_SUCCESS);
    assert(elem != NULL);
    assert(strcmp((const char *)elem->value, "one") == 0);

    rc = aws_hash_table_find(&table, "beta", &elem);
    assert(rc == AWS_OP_SUCCESS);
    assert(elem != NULL);
    assert(strcmp((const char *)elem->value, "two") == 0);

    /* Lookup miss: rc == AWS_OP_SUCCESS, but elem == NULL (this is the
     * documented contract — find never errors on miss). */
    elem = NULL;
    rc = aws_hash_table_find(&table, "missing", &elem);
    assert(rc == AWS_OP_SUCCESS);
    assert(elem == NULL);

    aws_hash_table_clean_up(&table);
    aws_common_library_clean_up();
    printf("aws_c_common hash_table test OK\n");
    return 0;
}
