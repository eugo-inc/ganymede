/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for uriparser: core API; calls uriParseSingleUriA(), uriUnescapeInPlaceA(), uriFreeUriMembersA(), uriUriStringToUnixFilenameA().
 */

#include <uriparser/Uri.h>

#include <stdio.h>
#include <string.h>

#define CHECK(condition)                                                                    \
    do {                                                                                    \
        if (!(condition)) {                                                                 \
            fprintf(stderr, "URIPARSER: FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                                       \
        }                                                                                   \
    } while (0)

static int range_equals(const UriTextRangeA *range, const char *expected) {
    size_t length = (size_t)(range->afterLast - range->first);
    return range->first != NULL && length == strlen(expected) && memcmp(range->first, expected, length) == 0;
}

int main(void) {
    UriUriA uri;
    const char *error_position = NULL;
    const char *text = "s3://bucket.example.com:9000/some%20dir/file.parquet?region=us-east-1#frag";
    CHECK(uriParseSingleUriA(&uri, text, &error_position) == URI_SUCCESS);
    CHECK(range_equals(&uri.scheme, "s3"));
    CHECK(range_equals(&uri.hostText, "bucket.example.com"));
    CHECK(range_equals(&uri.portText, "9000"));
    CHECK(range_equals(&uri.query, "region=us-east-1"));
    CHECK(range_equals(&uri.fragment, "frag"));
    CHECK(uri.pathHead != NULL && range_equals(&uri.pathHead->text, "some%20dir"));
    CHECK(uri.pathHead->next != NULL && range_equals(&uri.pathHead->next->text, "file.parquet"));

    char segment[32];
    memcpy(segment, "some%20dir", sizeof("some%20dir"));
    uriUnescapeInPlaceA(segment);
    CHECK(strcmp(segment, "some dir") == 0);
    uriFreeUriMembersA(&uri);

    char path[64];
    CHECK(uriUriStringToUnixFilenameA("file:///tmp/a%20b.txt", path) == URI_SUCCESS);
    CHECK(strcmp(path, "/tmp/a b.txt") == 0);

    CHECK(uriParseSingleUriA(&uri, "http://[::1", &error_position) != URI_SUCCESS);

    printf("URIPARSER: PASS %s parse / unescape / file path round trip\n", URI_VER_ANSI);
    return 0;
}
