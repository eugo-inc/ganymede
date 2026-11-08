/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libyaml: scanner; calls yaml_parser_initialize(), yaml_parser_set_input_string(), yaml_parser_scan(), yaml_token_delete().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <yaml.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char doc_text[] = "fruit: apple\n";

    yaml_parser_t parser;
    assert(yaml_parser_initialize(&parser) == 1);
    yaml_parser_set_input_string(&parser,
                                 (const unsigned char *)doc_text,
                                 sizeof(doc_text) - 1);

    int saw_stream_start = 0;
    int saw_stream_end = 0;
    int saw_key = 0;
    int saw_value = 0;
    int saw_scalar_fruit = 0;
    int saw_scalar_apple = 0;

    for (;;) {
        yaml_token_t token;
        assert(yaml_parser_scan(&parser, &token) == 1);

        switch (token.type) {
            case YAML_STREAM_START_TOKEN:
                saw_stream_start = 1;
                break;
            case YAML_STREAM_END_TOKEN:
                saw_stream_end = 1;
                break;
            case YAML_KEY_TOKEN:
                saw_key = 1;
                break;
            case YAML_VALUE_TOKEN:
                saw_value = 1;
                break;
            case YAML_SCALAR_TOKEN: {
                const char *v = (const char *)token.data.scalar.value;
                if (strcmp(v, "fruit") == 0) saw_scalar_fruit = 1;
                if (strcmp(v, "apple") == 0) saw_scalar_apple = 1;
                break;
            }
            default:
                break;
        }

        int done = (token.type == YAML_STREAM_END_TOKEN);
        yaml_token_delete(&token);
        if (done) break;
    }

    assert(saw_stream_start);
    assert(saw_stream_end);
    assert(saw_key);
    assert(saw_value);
    assert(saw_scalar_fruit);
    assert(saw_scalar_apple);

    yaml_parser_delete(&parser);

    printf("libyaml scanner-token test OK\n");
    return 0;
}
