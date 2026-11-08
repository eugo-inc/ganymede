/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for glib: variant; calls g_variant_new(), g_variant_is_of_type(), g_variant_get(), g_variant_get_size().
 */

#include <glib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    GVariant *v = g_variant_new("(isb)", (gint32)42, "answer", TRUE);
    assert(v != NULL);
    assert(g_variant_is_of_type(v, G_VARIANT_TYPE("(isb)")));

    /* Verify the tuple unpacks to the same values. */
    gint32 i = 0;
    const gchar *s = NULL;
    gboolean b = FALSE;
    g_variant_get(v, "(i&sb)", &i, &s, &b);
    assert(i == 42);
    assert(s != NULL);
    assert(strcmp(s, "answer") == 0);
    assert(b == TRUE);

    /* Serialize to raw bytes (GVariant's stable wire format). */
    gsize size = g_variant_get_size(v);
    assert(size > 0);
    gconstpointer data = g_variant_get_data(v);
    assert(data != NULL);

    GVariant *v2 = g_variant_new_from_data(
        G_VARIANT_TYPE("(isb)"), data, size, FALSE, NULL, NULL
    );
    assert(v2 != NULL);
    assert(g_variant_equal(v, v2));

    g_variant_unref(v);
    g_variant_unref(v2);

    const char payload[] = "demo-payload";
    GBytes *bytes = g_bytes_new(payload, sizeof(payload) - 1);
    assert(bytes != NULL);
    assert(g_bytes_get_size(bytes) == sizeof(payload) - 1);

    gsize out_size = 0;
    const guint8 *out = (const guint8 *)g_bytes_get_data(bytes, &out_size);
    assert(out_size == sizeof(payload) - 1);
    assert(memcmp(out, payload, out_size) == 0);

    /* Equality between two GBytes with identical content. */
    GBytes *bytes2 = g_bytes_new(payload, sizeof(payload) - 1);
    assert(g_bytes_equal(bytes, bytes2));

    g_bytes_unref(bytes);
    g_bytes_unref(bytes2);

    printf("glib GVariant/GBytes test OK\n");
    return 0;
}
