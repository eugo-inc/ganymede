/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxslt: exslt; calls exsltRegisterAll(), xmlReadMemory(), xsltParseStylesheetDoc(), xsltApplyStylesheet().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libexslt/exslt.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Register every EXSLT extension namespace (strings, math, date, etc.). */
    exsltRegisterAll();

    /* Stylesheet that uses str:tokenize from the EXSLT strings namespace.
     * Splits "a,b,c" on ',' and emits the resulting tokens as <t> children. */
    static const char *xsl_str =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\""
        " xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\""
        " xmlns:str=\"http://exslt.org/strings\""
        " extension-element-prefixes=\"str\">"
        "<xsl:template match=\"/\">"
        "<out><xsl:for-each select=\"str:tokenize('a,b,c', ',')\">"
        "<t><xsl:value-of select=\".\"/></t>"
        "</xsl:for-each></out>"
        "</xsl:template>"
        "</xsl:stylesheet>";
    static const char *xml_str =
        "<?xml version=\"1.0\"?><root/>";

    xmlDocPtr xsl_doc = xmlReadMemory(xsl_str, (int)strlen(xsl_str), "sheet.xsl", NULL, 0);
    assert(xsl_doc != NULL);
    xsltStylesheetPtr sheet = xsltParseStylesheetDoc(xsl_doc);
    assert(sheet != NULL);

    xmlDocPtr in_doc = xmlReadMemory(xml_str, (int)strlen(xml_str), "in.xml", NULL, 0);
    assert(in_doc != NULL);

    xmlDocPtr out_doc = xsltApplyStylesheet(sheet, in_doc, NULL);
    assert(out_doc != NULL);

    xmlNodePtr root = xmlDocGetRootElement(out_doc);
    assert(root != NULL);
    assert(xmlStrcmp(root->name, (const xmlChar *)"out") == 0);

    /* Verify str:tokenize produced exactly 3 <t> element children. */
    int t_count = 0;
    for (xmlNodePtr n = root->children; n != NULL; n = n->next) {
        if (n->type == XML_ELEMENT_NODE && xmlStrcmp(n->name, (const xmlChar *)"t") == 0) {
            t_count++;
        }
    }
    assert(t_count == 3);

    xmlFreeDoc(out_doc);
    xmlFreeDoc(in_doc);
    xsltFreeStylesheet(sheet);
    xsltCleanupGlobals();
    xmlCleanupParser();

    printf("libexslt EXSLT-strings test OK (tokens=%d)\n", t_count);
    return 0;
}
