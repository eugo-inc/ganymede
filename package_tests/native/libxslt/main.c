/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxslt: core API; calls xmlReadMemory(), xsltParseStylesheetDoc(), xsltApplyStylesheet(), xmlDocGetRootElement().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char *xsl_str =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
        "<xsl:template match=\"/\"><out><xsl:value-of select=\"root/child\"/></out></xsl:template>"
        "</xsl:stylesheet>";
    static const char *xml_str =
        "<?xml version=\"1.0\"?><root><child>hello</child></root>";

    xmlDocPtr xsl_doc = xmlReadMemory(xsl_str, (int)strlen(xsl_str), "sheet.xsl", NULL, 0);
    assert(xsl_doc != NULL);
    xsltStylesheetPtr sheet = xsltParseStylesheetDoc(xsl_doc);  /* takes ownership of xsl_doc */
    assert(sheet != NULL);

    xmlDocPtr in_doc = xmlReadMemory(xml_str, (int)strlen(xml_str), "in.xml", NULL, 0);
    assert(in_doc != NULL);

    xmlDocPtr out_doc = xsltApplyStylesheet(sheet, in_doc, NULL);
    assert(out_doc != NULL);

    xmlNodePtr root = xmlDocGetRootElement(out_doc);
    assert(root != NULL);
    assert(xmlStrcmp(root->name, (const xmlChar *)"out") == 0);

    xmlFreeDoc(out_doc);
    xmlFreeDoc(in_doc);
    xsltFreeStylesheet(sheet);  /* also frees xsl_doc */
    xsltCleanupGlobals();
    xmlCleanupParser();

    printf("libxslt test OK: %d\n", xsltLibxsltVersion);
    return 0;
}
