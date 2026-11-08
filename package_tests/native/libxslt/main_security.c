/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxslt: security; calls xsltSetGenericErrorFunc(), xmlReadMemory(), xsltParseStylesheetDoc(), xsltNewSecurityPrefs().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>
#include <libxslt/security.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Callback signature matches xsltSecurityCheck in libxslt/security.h:
 *   typedef int (*xsltSecurityCheck)(xsltSecurityPrefsPtr sec,
 *                                    xsltTransformContextPtr ctxt,
 *                                    const char *value);
 * Return 0 to forbid, 1 to allow. */
static int g_read_check_fired = 0;
static int deny_read_file_cb(xsltSecurityPrefsPtr sec,
                             xsltTransformContextPtr ctxt,
                             const char *value) {
    (void)sec;
    (void)ctxt;
    (void)value;
    g_read_check_fired = 1;
    return 0; /* forbid */
}

int main(void) {
    /* Stylesheet that uses document() to pull /etc/hosts. The custom
     * security callback should fire when the document() call attempts
     * to read the file. */
    static const char *xsl_str =
        "<?xml version=\"1.0\"?>"
        "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
        "<xsl:template match=\"/\">"
        "<out><xsl:value-of select=\"count(document('/etc/hosts'))\"/></out>"
        "</xsl:template>"
        "</xsl:stylesheet>";
    static const char *xml_str =
        "<?xml version=\"1.0\"?><root/>";

    /* Silence libxslt's stderr "runtime error" chatter — the denied read
     * is the WHOLE POINT of this test, so the diagnostic noise is not a
     * failure signal. */
    xsltSetGenericErrorFunc(NULL, NULL);

    xmlDocPtr xsl_doc = xmlReadMemory(xsl_str, (int)strlen(xsl_str), "sheet.xsl", NULL, 0);
    assert(xsl_doc != NULL);
    xsltStylesheetPtr sheet = xsltParseStylesheetDoc(xsl_doc);
    assert(sheet != NULL);

    xmlDocPtr in_doc = xmlReadMemory(xml_str, (int)strlen(xml_str), "in.xml", NULL, 0);
    assert(in_doc != NULL);

    xsltSecurityPrefsPtr sec = xsltNewSecurityPrefs();
    assert(sec != NULL);
    int rc;
    rc = xsltSetSecurityPrefs(sec, XSLT_SECPREF_READ_FILE, deny_read_file_cb);
    assert(rc == 0);
    rc = xsltSetSecurityPrefs(sec, XSLT_SECPREF_WRITE_FILE, xsltSecurityForbid);
    assert(rc == 0);
    rc = xsltSetSecurityPrefs(sec, XSLT_SECPREF_CREATE_DIRECTORY, xsltSecurityForbid);
    assert(rc == 0);
    rc = xsltSetSecurityPrefs(sec, XSLT_SECPREF_READ_NETWORK, xsltSecurityForbid);
    assert(rc == 0);
    rc = xsltSetSecurityPrefs(sec, XSLT_SECPREF_WRITE_NETWORK, xsltSecurityForbid);
    assert(rc == 0);

    xsltTransformContextPtr ctxt = xsltNewTransformContext(sheet, in_doc);
    assert(ctxt != NULL);
    rc = xsltSetCtxtSecurityPrefs(sec, ctxt);
    assert(rc == 0);

    xmlDocPtr out_doc = xsltApplyStylesheetUser(sheet, in_doc, NULL, NULL, NULL, ctxt);

    assert(g_read_check_fired == 1);

    if (out_doc != NULL) {
        xmlFreeDoc(out_doc);
    }
    xsltFreeTransformContext(ctxt);
    xsltFreeSecurityPrefs(sec);
    xmlFreeDoc(in_doc);
    xsltFreeStylesheet(sheet);
    xsltCleanupGlobals();
    xmlCleanupParser();

    printf("libxslt security-prefs sandbox test OK\n");
    return 0;
}
