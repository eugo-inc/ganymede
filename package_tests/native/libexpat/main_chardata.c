/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libexpat: chardata; calls XML_ParserCreate(), XML_SetCharacterDataHandler(), XML_Parse(), XML_GetCurrentByteIndex().
 */

#include <expat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static char g_buf[256];
static int g_len = 0;

static void XMLCALL on_chardata(void *user_data, const XML_Char *s, int len) {
    (void)user_data;
    assert(len > 0);
    assert(g_len + len < (int)sizeof(g_buf));
    memcpy(g_buf + g_len, s, (size_t)len);
    g_len += len;
}

int main(void) {
    XML_Parser parser = XML_ParserCreate(NULL);
    assert(parser != NULL);

    XML_SetCharacterDataHandler(parser, on_chardata);

    static const char doc[] = "<root>hello world</root>";
    enum XML_Status rc = XML_Parse(parser, doc, (int)(sizeof(doc) - 1), /*isFinal=*/1);
    assert(rc == XML_STATUS_OK);

    g_buf[g_len] = '\0';
    assert(g_len == 11);
    assert(strcmp(g_buf, "hello world") == 0);

    XML_Index idx = XML_GetCurrentByteIndex(parser);
    assert(idx >= 0);

    XML_ParserFree(parser);

    printf("libexpat chardata test OK: captured %d bytes\n", g_len);
    return 0;
}
