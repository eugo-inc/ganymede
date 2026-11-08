/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libexpat: error; calls XML_ParserCreate(), XML_Parse(), XML_GetErrorCode(), XML_ErrorString().
 */

#include <expat.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    XML_Parser parser = XML_ParserCreate(NULL);
    assert(parser != NULL);

    /* Mismatched tags — well-formedness violation, not just a schema gripe. */
    static const char bad[] = "<root><child></wrong></root>";
    enum XML_Status rc = XML_Parse(parser, bad, (int)(sizeof(bad) - 1), /*isFinal=*/1);
    assert(rc == XML_STATUS_ERROR);

    enum XML_Error code = XML_GetErrorCode(parser);
    assert(code != XML_ERROR_NONE);
    assert(code == XML_ERROR_TAG_MISMATCH);

    const XML_LChar *msg = XML_ErrorString(code);
    assert(msg != NULL);
    assert(strlen(msg) > 0);

    XML_Size line = XML_GetCurrentLineNumber(parser);
    assert(line >= 1);

    XML_ParserFree(parser);

    printf("libexpat error test OK: code=%d msg=\"%s\" line=%lu\n",
           (int)code, msg, (unsigned long)line);
    return 0;
}
