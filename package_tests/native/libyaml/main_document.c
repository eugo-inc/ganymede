/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libyaml: document; calls yaml_parser_initialize(), yaml_parser_set_input_string(), yaml_parser_load(), yaml_document_get_root_node().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <yaml.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char doc_text[] =
        "name: demo-document\n"
        "ok: true\n"
        "count: 42\n";

    yaml_parser_t parser;
    assert(yaml_parser_initialize(&parser) == 1);
    yaml_parser_set_input_string(&parser,
                                 (const unsigned char *)doc_text,
                                 sizeof(doc_text) - 1);

    yaml_document_t document;
    assert(yaml_parser_load(&parser, &document) == 1);

    yaml_node_t *root = yaml_document_get_root_node(&document);
    assert(root != NULL);
    assert(root->type == YAML_MAPPING_NODE);

    /* Walk the mapping; locate the key "name" and assert its value scalar. */
    int found_name = 0;
    int found_count = 0;
    yaml_node_pair_t *pair;
    for (pair = root->data.mapping.pairs.start;
         pair < root->data.mapping.pairs.top;
         pair++) {
        yaml_node_t *key_node = yaml_document_get_node(&document, pair->key);
        yaml_node_t *val_node = yaml_document_get_node(&document, pair->value);
        assert(key_node != NULL);
        assert(val_node != NULL);
        assert(key_node->type == YAML_SCALAR_NODE);
        assert(val_node->type == YAML_SCALAR_NODE);

        const char *key = (const char *)key_node->data.scalar.value;
        const char *val = (const char *)val_node->data.scalar.value;

        if (strcmp(key, "name") == 0) {
            assert(strcmp(val, "demo-document") == 0);
            found_name = 1;
        }
        if (strcmp(key, "count") == 0) {
            assert(strcmp(val, "42") == 0);
            found_count = 1;
        }
    }
    assert(found_name);
    assert(found_count);

    yaml_document_delete(&document);
    yaml_parser_delete(&parser);

    printf("libyaml document-tree test OK\n");
    return 0;
}
