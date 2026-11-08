/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for glib: core API; calls glib_check_version(), g_string_new(), g_string_append(), g_string_free().
 */

#include <glib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Compile-time vs runtime version agreement — NULL means "compatible". */
    const char *vmismatch = glib_check_version(
        GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION
    );
    assert(vmismatch == NULL);

    GString *s = g_string_new("hello");
    assert(s != NULL);
    g_string_append(s, ", world");
    assert(s->len == 12);
    assert(strcmp(s->str, "hello, world") == 0);
    g_string_free(s, TRUE);

    GHashTable *t = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);
    assert(t != NULL);
    g_hash_table_insert(t, (gpointer)"alpha", (gpointer)"A");
    g_hash_table_insert(t, (gpointer)"beta", (gpointer)"B");
    assert(g_hash_table_size(t) == 2);
    const char *got = (const char *)g_hash_table_lookup(t, "alpha");
    assert(got != NULL);
    assert(strcmp(got, "A") == 0);
    g_hash_table_destroy(t);

    printf("glib test OK: %d.%d.%d\n",
           glib_major_version, glib_minor_version, glib_micro_version);
    return 0;
}
