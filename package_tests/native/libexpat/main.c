/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libexpat: core API; calls XML_ParserCreate(), XML_SetElementHandler(), XML_Parse(), XML_ParserFree().
 */

#include <expat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static int g_start_count = 0;

static void XMLCALL on_start(void *user_data, const XML_Char *name, const XML_Char **attrs) {
    (void)user_data;
    (void)attrs;
    if (strcmp(name, "root") == 0 || strcmp(name, "child") == 0) {
        g_start_count++;
    }
}

int main(void) {
    XML_Parser parser = XML_ParserCreate(NULL);
    assert(parser != NULL);

    XML_SetElementHandler(parser, on_start, NULL);

    static const char doc[] = "<root><child/><child/></root>";
    enum XML_Status rc = XML_Parse(parser, doc, (int)(sizeof(doc) - 1), /*isFinal=*/1);
    assert(rc == XML_STATUS_OK);
    assert(g_start_count == 3);

    XML_ParserFree(parser);

    const XML_LChar *version = XML_ExpatVersion();
    assert(version != NULL);
    assert(strstr(version, "expat_") != NULL);

    printf("libexpat test OK: %s\n", version);
    return 0;
}
