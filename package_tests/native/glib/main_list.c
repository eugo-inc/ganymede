/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for glib: list; calls g_list_append(), g_list_length(), g_list_reverse(), g_list_last().
 */

#include <glib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static gint cmp_str(gconstpointer a, gconstpointer b) {
    return strcmp((const char *)a, (const char *)b);
}

int main(void) {
    /* GList: append yields stable order; length is O(n) and must agree. */
    GList *list = NULL;
    list = g_list_append(list, (gpointer)"one");
    list = g_list_append(list, (gpointer)"two");
    list = g_list_append(list, (gpointer)"three");
    assert(g_list_length(list) == 3);
    assert(strcmp((const char *)list->data, "one") == 0);
    assert(strcmp((const char *)list->next->data, "two") == 0);
    assert(strcmp((const char *)list->next->next->data, "three") == 0);
    assert(list->next->next->next == NULL);

    /* Reverse: doubly-linked traversal both directions. */
    list = g_list_reverse(list);
    assert(strcmp((const char *)list->data, "three") == 0);
    assert(strcmp((const char *)g_list_last(list)->data, "one") == 0);

    /* Find with custom comparator. */
    GList *hit = g_list_find_custom(list, "two", cmp_str);
    assert(hit != NULL);
    assert(strcmp((const char *)hit->data, "two") == 0);

    g_list_free(list);

    /* GSList: prepend is the canonical O(1) op; verify chain order. */
    GSList *slist = NULL;
    slist = g_slist_prepend(slist, (gpointer)"gamma");
    slist = g_slist_prepend(slist, (gpointer)"beta");
    slist = g_slist_prepend(slist, (gpointer)"alpha");
    assert(g_slist_length(slist) == 3);
    assert(strcmp((const char *)slist->data, "alpha") == 0);

    /* Sort: in-place merge sort with comparator. */
    slist = g_slist_sort(slist, cmp_str);
    assert(strcmp((const char *)slist->data, "alpha") == 0);
    assert(strcmp((const char *)slist->next->data, "beta") == 0);
    assert(strcmp((const char *)slist->next->next->data, "gamma") == 0);

    g_slist_free(slist);

    printf("glib GList/GSList test OK\n");
    return 0;
}
