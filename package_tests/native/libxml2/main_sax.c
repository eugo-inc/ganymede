/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxml2: sax; calls xmlStrcmp(), xmlSAXUserParseMemory(), xmlCleanupParser().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/SAX2.h>
#include <libxml/xmlversion.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    int start_count;
    int end_count;
    int chars_total;
    int saw_root;
    int saw_leaf;
} Counters;

static void on_start(void *user_data, const xmlChar *name, const xmlChar **attrs) {
    (void)attrs;
    Counters *c = (Counters *)user_data;
    c->start_count++;
    if (xmlStrcmp(name, (const xmlChar *)"root") == 0) {
        c->saw_root = 1;
    } else if (xmlStrcmp(name, (const xmlChar *)"leaf") == 0) {
        c->saw_leaf = 1;
    }
}

static void on_end(void *user_data, const xmlChar *name) {
    (void)name;
    Counters *c = (Counters *)user_data;
    c->end_count++;
}

static void on_chars(void *user_data, const xmlChar *ch, int len) {
    (void)ch;
    Counters *c = (Counters *)user_data;
    c->chars_total += len;
}

int main(void) {
    LIBXML_TEST_VERSION

    static const char *doc_str =
        "<?xml version=\"1.0\"?>"
        "<root>"
            "<leaf>abc</leaf>"
            "<leaf>defgh</leaf>"
        "</root>";

    xmlSAXHandler handler;
    memset(&handler, 0, sizeof(handler));
    handler.initialized = XML_SAX2_MAGIC;
    handler.startElement = on_start;
    handler.endElement = on_end;
    handler.characters = on_chars;

    Counters counters = {0, 0, 0, 0, 0};

    int rc = xmlSAXUserParseMemory(&handler, &counters, doc_str, (int)strlen(doc_str));
    assert(rc == 0);

    /* 1 root + 2 leaf = 3 start/end pairs. */
    assert(counters.start_count == 3);
    assert(counters.end_count == 3);
    assert(counters.saw_root == 1);
    assert(counters.saw_leaf == 1);
    /* "abc" (3) + "defgh" (5) = 8 chars of text content. */
    assert(counters.chars_total == 8);

    xmlCleanupParser();

    printf("libxml2 sax test OK (starts=%d ends=%d chars=%d)\n",
           counters.start_count, counters.end_count, counters.chars_total);
    return 0;
}
