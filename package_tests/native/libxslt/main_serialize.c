/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxslt: serialize; calls xmlReadMemory(), xsltParseStylesheetDoc(), xsltApplyStylesheet(), xsltSaveResultToString().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* xsl:output method="text" — the serializer should emit ONLY the value-of
     * content, no XML wrapper. */
    static const char *xsl_str =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
        "<xsl:output method=\"text\" encoding=\"UTF-8\"/>"
        "<xsl:template match=\"/\">"
        "<xsl:value-of select=\"root/child\"/>"
        "</xsl:template>"
        "</xsl:stylesheet>";
    static const char *xml_str =
        "<?xml version=\"1.0\"?><root><child>serialized-payload</child></root>";

    xmlDocPtr xsl_doc = xmlReadMemory(xsl_str, (int)strlen(xsl_str), "sheet.xsl", NULL, 0);
    assert(xsl_doc != NULL);
    xsltStylesheetPtr sheet = xsltParseStylesheetDoc(xsl_doc);
    assert(sheet != NULL);

    xmlDocPtr in_doc = xmlReadMemory(xml_str, (int)strlen(xml_str), "in.xml", NULL, 0);
    assert(in_doc != NULL);

    xmlDocPtr out_doc = xsltApplyStylesheet(sheet, in_doc, NULL);
    assert(out_doc != NULL);

    /* Serialize through the stylesheet's output method (text). */
    xmlChar *buf = NULL;
    int buf_len = 0;
    int rc = xsltSaveResultToString(&buf, &buf_len, out_doc, sheet);
    assert(rc == 0);
    assert(buf != NULL);
    assert(buf_len > 0);

    /* With method="text", the output must NOT contain XML tags AND must contain
     * the child element's text value verbatim. */
    assert(strstr((const char *)buf, "<") == NULL);
    assert(strstr((const char *)buf, "serialized-payload") != NULL);

    xmlFree(buf);
    xmlFreeDoc(out_doc);
    xmlFreeDoc(in_doc);
    xsltFreeStylesheet(sheet);
    xsltCleanupGlobals();
    xmlCleanupParser();

    printf("libxslt serializer test OK (output method=text, %d bytes)\n", buf_len);
    return 0;
}
