/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for http_parser: url; calls http_parser_url_init(), http_parser_parse_url(), http_parser_version().
 */

#include <http_parser.h>

#include <stdio.h>
#include <string.h>

static int field_equals(const char *url, const struct http_parser_url *parsed, enum http_parser_url_fields field,
                        const char *expected) {
    if ((parsed->field_set & (1u << field)) == 0) {
        return 0;
    }
    const size_t length = parsed->field_data[field].len;
    return length == strlen(expected) && memcmp(url + parsed->field_data[field].off, expected, length) == 0;
}

int main(void) {
    const char *url = "http://example.com:8080/api/items?limit=5";
    struct http_parser_url parsed;
    http_parser_url_init(&parsed);

    if (http_parser_parse_url(url, strlen(url), 0, &parsed) != 0) {
        fprintf(stderr, "http_parser_parse_url rejected %s\n", url);
        return 1;
    }
    if (!field_equals(url, &parsed, UF_HOST, "example.com") || !field_equals(url, &parsed, UF_PATH, "/api/items") ||
        !field_equals(url, &parsed, UF_QUERY, "limit=5") || parsed.port != 8080) {
        fprintf(stderr, "http_parser_parse_url split %s wrongly (port=%u)\n", url, (unsigned)parsed.port);
        return 1;
    }

    const unsigned long version = http_parser_version();
    const unsigned major = (unsigned)((version >> 16) & 255u);
    const unsigned minor = (unsigned)((version >> 8) & 255u);
    const unsigned patch = (unsigned)(version & 255u);
    if (major != HTTP_PARSER_VERSION_MAJOR || minor != HTTP_PARSER_VERSION_MINOR) {
        fprintf(stderr, "header says %d.%d but the library says %u.%u\n", HTTP_PARSER_VERSION_MAJOR,
                HTTP_PARSER_VERSION_MINOR, major, minor);
        return 1;
    }

    printf("http_parser %u.%u.%u: parsed host, port, path and query\n", major, minor, patch);
    return 0;
}
