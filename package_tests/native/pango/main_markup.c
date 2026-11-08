/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pango: markup; calls pango_parse_markup(), pango_attr_list_get_iterator(), pango_attr_iterator_get(), pango_attr_iterator_next().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <pango/pango.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *markup = "<b>Hello</b> <i>world</i>";
    PangoAttrList *attrs = NULL;
    char *plain = NULL;
    GError *err = NULL;

    gboolean ok = pango_parse_markup(markup, -1, 0, &attrs, &plain, NULL, &err);
    assert(ok);
    assert(err == NULL);
    assert(plain != NULL);
    assert(strcmp(plain, "Hello world") == 0);
    assert(attrs != NULL);

    /* Walk the attribute list and confirm at least one weight + one style
     * attribute were emitted (from <b> and <i>). */
    PangoAttrIterator *it = pango_attr_list_get_iterator(attrs);
    assert(it != NULL);
    int saw_bold = 0;
    int saw_italic = 0;
    do {
        PangoAttribute *w = pango_attr_iterator_get(it, PANGO_ATTR_WEIGHT);
        PangoAttribute *s = pango_attr_iterator_get(it, PANGO_ATTR_STYLE);
        if (w != NULL) {
            PangoAttrInt *wi = (PangoAttrInt *)w;
            if (wi->value == PANGO_WEIGHT_BOLD) saw_bold = 1;
        }
        if (s != NULL) {
            PangoAttrInt *si = (PangoAttrInt *)s;
            if (si->value == PANGO_STYLE_ITALIC) saw_italic = 1;
        }
    } while (pango_attr_iterator_next(it));
    assert(saw_bold);
    assert(saw_italic);
    pango_attr_iterator_destroy(it);

    g_free(plain);
    pango_attr_list_unref(attrs);

    /* Foreground-color span via the foreground= attribute. */
    const char *fg_markup = "<span foreground=\"#ff0000\">red</span>";
    PangoAttrList *fg_attrs = NULL;
    char *fg_plain = NULL;
    ok = pango_parse_markup(fg_markup, -1, 0, &fg_attrs, &fg_plain, NULL, NULL);
    assert(ok);
    assert(fg_plain != NULL);
    assert(strcmp(fg_plain, "red") == 0);
    assert(fg_attrs != NULL);
    g_free(fg_plain);
    pango_attr_list_unref(fg_attrs);

    GError *err2 = NULL;
    PangoAttrList *bad_attrs = NULL;
    char *bad_plain = NULL;
    gboolean bad_ok = pango_parse_markup("<b>unclosed", -1, 0,
                                          &bad_attrs, &bad_plain, NULL, &err2);
    assert(!bad_ok);
    assert(err2 != NULL);
    assert(err2->message != NULL);
    g_error_free(err2);

    GError *err3 = NULL;
    PangoAttrList *acc_attrs = NULL;
    char *acc_plain = NULL;
    gunichar accel = 0;
    ok = pango_parse_markup("_File", -1, '_', &acc_attrs, &acc_plain,
                            &accel, &err3);
    assert(ok);
    assert(err3 == NULL);
    assert(acc_plain != NULL);
    assert(strcmp(acc_plain, "File") == 0);
    assert(accel == 'F');
    g_free(acc_plain);
    if (acc_attrs != NULL) pango_attr_list_unref(acc_attrs);

    printf("pango markup test OK\n");
    return 0;
}
