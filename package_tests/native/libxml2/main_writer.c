/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libxml2: writer; calls xmlBufferCreate(), xmlNewTextWriterMemory(), xmlTextWriterStartDocument(), xmlTextWriterStartElement().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlversion.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    LIBXML_TEST_VERSION

    xmlBufferPtr buf = xmlBufferCreate();
    assert(buf != NULL);

    xmlTextWriterPtr writer = xmlNewTextWriterMemory(buf, 0);
    assert(writer != NULL);

    int rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    assert(rc >= 0);

    rc = xmlTextWriterStartElement(writer, (const xmlChar *)"items");
    assert(rc >= 0);

    rc = xmlTextWriterStartElement(writer, (const xmlChar *)"item");
    assert(rc >= 0);
    rc = xmlTextWriterWriteAttribute(writer, (const xmlChar *)"key",
                                     (const xmlChar *)"alpha");
    assert(rc >= 0);
    rc = xmlTextWriterWriteString(writer, (const xmlChar *)"value-1");
    assert(rc >= 0);
    rc = xmlTextWriterEndElement(writer);
    assert(rc >= 0);

    rc = xmlTextWriterEndElement(writer); /* </items> */
    assert(rc >= 0);
    rc = xmlTextWriterEndDocument(writer);
    assert(rc >= 0);

    xmlFreeTextWriter(writer);

    /* The buffer must now contain a non-trivial XML document. */
    const char *out = (const char *)xmlBufferContent(buf);
    assert(out != NULL);
    int out_len = xmlBufferLength(buf);
    assert(out_len > 0);
    assert(strstr(out, "<items>") != NULL);
    assert(strstr(out, "key=\"alpha\"") != NULL);
    assert(strstr(out, "value-1") != NULL);

    xmlDocPtr doc = xmlReadMemory(out, out_len, "writer.xml", NULL, 0);
    assert(doc != NULL);
    xmlNodePtr root = xmlDocGetRootElement(doc);
    assert(root != NULL);
    assert(xmlStrcmp(root->name, (const xmlChar *)"items") == 0);

    xmlNodePtr child = root->children;
    /* Skip any text nodes that may appear (none expected without indent, but be safe). */
    while (child != NULL && child->type != XML_ELEMENT_NODE) {
        child = child->next;
    }
    assert(child != NULL);
    assert(xmlStrcmp(child->name, (const xmlChar *)"item") == 0);

    xmlChar *attr = xmlGetProp(child, (const xmlChar *)"key");
    assert(attr != NULL);
    assert(xmlStrcmp(attr, (const xmlChar *)"alpha") == 0);
    xmlFree(attr);

    xmlFreeDoc(doc);
    xmlBufferFree(buf);
    xmlCleanupParser();

    printf("libxml2 writer test OK (%d bytes)\n", out_len);
    return 0;
}
