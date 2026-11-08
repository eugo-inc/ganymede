/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libexpat: attrs; calls XML_ParserCreate(), XML_SetStartElementHandler(), XML_Parse(), XML_ParserFree().
 */

#include <expat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int g_id_seen = 0;
static int g_kind_seen = 0;
static int g_attr_count = 0;

static void XMLCALL on_start(void *user_data, const XML_Char *name, const XML_Char **attrs) {
    (void)user_data;
    if (strcmp(name, "item") != 0) {
        return;
    }
    for (int i = 0; attrs[i] != NULL; i += 2) {
        assert(attrs[i + 1] != NULL); /* attrs come in (name, value) pairs */
        g_attr_count++;
        if (strcmp(attrs[i], "id") == 0 && strcmp(attrs[i + 1], "42") == 0) {
            g_id_seen = 1;
        } else if (strcmp(attrs[i], "kind") == 0 && strcmp(attrs[i + 1], "widget") == 0) {
            g_kind_seen = 1;
        }
    }
}

int main(void) {
    XML_Parser parser = XML_ParserCreate(NULL);
    assert(parser != NULL);

    XML_SetStartElementHandler(parser, on_start);

    static const char doc[] = "<root><item id=\"42\" kind=\"widget\"/></root>";
    enum XML_Status rc = XML_Parse(parser, doc, (int)(sizeof(doc) - 1), /*isFinal=*/1);
    assert(rc == XML_STATUS_OK);
    assert(g_attr_count == 2);
    assert(g_id_seen == 1);
    assert(g_kind_seen == 1);

    XML_ParserFree(parser);

    printf("libexpat attrs test OK: id+kind round-tripped\n");
    return 0;
}
