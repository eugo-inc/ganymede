/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxml2: core API; calls xmlReadMemory(), xmlDocGetRootElement(), xmlStrcmp(), xmlFreeDoc().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlversion.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    LIBXML_TEST_VERSION

    static const char *doc_str =
        "<?xml version=\"1.0\"?><root><child>hello</child></root>";

    xmlDocPtr doc = xmlReadMemory(doc_str, (int)strlen(doc_str), "noname.xml", NULL, 0);
    assert(doc != NULL);

    xmlNodePtr root = xmlDocGetRootElement(doc);
    assert(root != NULL);
    assert(xmlStrcmp(root->name, (const xmlChar *)"root") == 0);

    xmlFreeDoc(doc);
    xmlCleanupParser();

    printf("libxml2 test OK: %s\n", LIBXML_DOTTED_VERSION);
    return 0;
}
