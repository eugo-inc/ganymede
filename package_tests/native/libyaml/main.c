/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libyaml: core API; calls yaml_get_version_string(), yaml_get_version(), yaml_parser_initialize(), yaml_parser_set_input_string().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <yaml.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *v = yaml_get_version_string();
    assert(v != NULL);

    int major = 0, minor = 0, patch = 0;
    yaml_get_version(&major, &minor, &patch);
    assert(major >= 0 && minor >= 0 && patch >= 0);

    static const char doc[] = "key: value\nlist:\n  - 1\n  - 2\n";

    yaml_parser_t parser;
    assert(yaml_parser_initialize(&parser) == 1);
    yaml_parser_set_input_string(&parser, (const unsigned char *)doc, sizeof(doc) - 1);

    int saw_stream_start = 0;
    int saw_stream_end = 0;
    int saw_scalar_value = 0;
    for (;;) {
        yaml_event_t event;
        assert(yaml_parser_parse(&parser, &event) == 1);
        if (event.type == YAML_STREAM_START_EVENT) saw_stream_start = 1;
        if (event.type == YAML_SCALAR_EVENT &&
            strcmp((const char *)event.data.scalar.value, "value") == 0) {
            saw_scalar_value = 1;
        }
        int done = (event.type == YAML_STREAM_END_EVENT);
        if (done) saw_stream_end = 1;
        yaml_event_delete(&event);
        if (done) break;
    }
    assert(saw_stream_start && saw_stream_end && saw_scalar_value);

    yaml_parser_delete(&parser);

    printf("libyaml test OK: %s\n", v);
    return 0;
}
