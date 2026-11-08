/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxslt: extfunc; calls xmlXPathPopString(), xmlStrlen(), xmlMalloc(), xmlFree().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/extensions.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void demo_ext_extend(xmlXPathParserContextPtr ctxt, int nargs) {
    assert(nargs == 1);
    xmlChar *arg = xmlXPathPopString(ctxt);
    assert(arg != NULL);

    size_t arg_len = xmlStrlen(arg);
    static const char suffix[] = "-extended";
    xmlChar *out = xmlMalloc(arg_len + sizeof(suffix));
    assert(out != NULL);
    memcpy(out, arg, arg_len);
    memcpy(out + arg_len, suffix, sizeof(suffix));  /* includes the NUL */

    xmlFree(arg);
    xmlXPathReturnString(ctxt, out);  /* takes ownership of `out` */
}

int main(void) {
    static const xmlChar *ns_uri = (const xmlChar *)"http://example.org/ext";
    static const xmlChar *fn_name = (const xmlChar *)"extend";

    int rc = xsltRegisterExtModuleFunction(fn_name, ns_uri, demo_ext_extend);
    assert(rc == 0);

    static const char *xsl_str =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\""
        " xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\""
        " xmlns:demo=\"http://example.org/ext\""
        " extension-element-prefixes=\"demo\">"
        "<xsl:template match=\"/\">"
        "<out><xsl:value-of select=\"demo:extend('hello')\"/></out>"
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

    xmlChar *text = xmlNodeGetContent(root);
    assert(text != NULL);
    assert(xmlStrcmp(text, (const xmlChar *)"hello-extended") == 0);

    xmlFree(text);
    xmlFreeDoc(out_doc);
    xmlFreeDoc(in_doc);
    xsltFreeStylesheet(sheet);
    xsltCleanupGlobals();
    xmlCleanupParser();

    printf("libxslt extension-function test OK\n");
    return 0;
}
