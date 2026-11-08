/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pango: attr; calls pango_attr_list_new(), pango_attr_foreground_new(), pango_attr_weight_new(), pango_attr_size_new().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    PangoAttrList *list = pango_attr_list_new();
    assert(list != NULL);

    PangoAttribute *attr_fg = pango_attr_foreground_new(0xffff, 0x0000, 0x0000);
    assert(attr_fg != NULL);
    attr_fg->start_index = 0;
    attr_fg->end_index = 5;

    PangoAttribute *attr_weight = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    assert(attr_weight != NULL);
    attr_weight->start_index = 0;
    attr_weight->end_index = 10;

    PangoAttribute *attr_size = pango_attr_size_new(12 * PANGO_SCALE);
    assert(attr_size != NULL);
    attr_size->start_index = 5;
    attr_size->end_index = 10;

    /* Sanity-check the klass tagging — distinct types must have distinct klass ptrs. */
    assert(attr_fg->klass != NULL);
    assert(attr_weight->klass != NULL);
    assert(attr_size->klass != NULL);
    assert(attr_fg->klass->type == PANGO_ATTR_FOREGROUND);
    assert(attr_weight->klass->type == PANGO_ATTR_WEIGHT);
    assert(attr_size->klass->type == PANGO_ATTR_SIZE);

    pango_attr_list_insert(list, attr_fg);
    pango_attr_list_insert(list, attr_weight);
    pango_attr_list_insert(list, attr_size);

    PangoAttrIterator *it = pango_attr_list_get_iterator(list);
    assert(it != NULL);

    int saw_weight = 0;
    int saw_foreground = 0;
    int saw_size = 0;
    do {
        PangoAttribute *got_weight = pango_attr_iterator_get(it, PANGO_ATTR_WEIGHT);
        PangoAttribute *got_fg = pango_attr_iterator_get(it, PANGO_ATTR_FOREGROUND);
        PangoAttribute *got_size = pango_attr_iterator_get(it, PANGO_ATTR_SIZE);
        if (got_weight != NULL) saw_weight = 1;
        if (got_fg != NULL) saw_foreground = 1;
        if (got_size != NULL) saw_size = 1;
    } while (pango_attr_iterator_next(it));

    assert(saw_weight);
    assert(saw_foreground);
    assert(saw_size);

    pango_attr_iterator_destroy(it);
    pango_attr_list_unref(list);

    printf("pango attr test OK\n");
    return 0;
}
