/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxml2: xpath; calls xmlReadMemory(), xmlXPathNewContext(), xmlXPathEvalExpression(), xmlNodeGetContent().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxml/xmlversion.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    LIBXML_TEST_VERSION

    static const char *doc_str =
        "<?xml version=\"1.0\"?>"
        "<catalog>"
            "<book id=\"a\"><title>Alpha</title></book>"
            "<book id=\"b\"><title>Beta</title></book>"
            "<book id=\"c\"><title>Gamma</title></book>"
        "</catalog>";

    xmlDocPtr doc = xmlReadMemory(doc_str, (int)strlen(doc_str), "xpath.xml", NULL, 0);
    assert(doc != NULL);

    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    assert(ctx != NULL);

    /* Pick the title of the book with id="b". */
    xmlXPathObjectPtr obj = xmlXPathEvalExpression(
        (const xmlChar *)"/catalog/book[@id='b']/title", ctx);
    assert(obj != NULL);
    assert(obj->type == XPATH_NODESET);
    assert(obj->nodesetval != NULL);
    assert(obj->nodesetval->nodeNr == 1);

    xmlNodePtr title = obj->nodesetval->nodeTab[0];
    assert(title != NULL);
    xmlChar *text = xmlNodeGetContent(title);
    assert(text != NULL);
    assert(xmlStrcmp(text, (const xmlChar *)"Beta") == 0);
    xmlFree(text);

    /* Sanity: a no-match query must yield an empty (but non-null) nodeset. */
    xmlXPathObjectPtr none = xmlXPathEvalExpression(
        (const xmlChar *)"/catalog/book[@id='zzz']/title", ctx);
    assert(none != NULL);
    assert(none->type == XPATH_NODESET);
    assert(none->nodesetval == NULL || none->nodesetval->nodeNr == 0);

    xmlXPathFreeObject(none);
    xmlXPathFreeObject(obj);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    printf("libxml2 xpath test OK\n");
    return 0;
}
