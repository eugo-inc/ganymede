/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for yajl: tree; calls yajl_version(), yajl_tree_parse(), yajl_tree_get(), yajl_tree_free().
 */

#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
#include <yajl/yajl_version.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    if (yajl_version() != YAJL_VERSION) {
        fprintf(stderr, "yajl_version.h says %d but the library says %d\n", YAJL_VERSION, yajl_version());
        return 1;
    }

    char error[256] = {0};
    yajl_val root = yajl_tree_parse("{\"name\":\"get_weather\",\"arguments\":[1,2,3]}", error, sizeof error);
    if (root == NULL) {
        fprintf(stderr, "yajl_tree_parse: %s\n", error);
        return 1;
    }
    const char *name_path[] = {"name", NULL};
    const char *arguments_path[] = {"arguments", NULL};
    yajl_val name = yajl_tree_get(root, name_path, yajl_t_string);
    yajl_val arguments = yajl_tree_get(root, arguments_path, yajl_t_array);
    const int tree_ok = name != NULL && strcmp(YAJL_GET_STRING(name), "get_weather") == 0 && arguments != NULL &&
                        YAJL_GET_ARRAY(arguments)->len == 3;
    yajl_tree_free(root);
    if (!tree_ok) {
        fprintf(stderr, "yajl_tree_get read back the wrong values\n");
        return 1;
    }

    yajl_gen generator = yajl_gen_alloc(NULL);
    if (generator == NULL) {
        fprintf(stderr, "yajl_gen_alloc returned NULL\n");
        return 1;
    }
    yajl_gen_map_open(generator);
    yajl_gen_string(generator, (const unsigned char *)"k", 1);
    yajl_gen_integer(generator, 7);
    yajl_gen_map_close(generator);
    const unsigned char *json = NULL;
    size_t json_length = 0;
    yajl_gen_get_buf(generator, &json, &json_length);
    const int generated_ok = json_length == 7 && memcmp(json, "{\"k\":7}", 7) == 0;
    yajl_gen_free(generator);
    if (!generated_ok) {
        fprintf(stderr, "yajl_gen produced unexpected output\n");
        return 1;
    }

    printf("yajl %d: tree parse + generator OK\n", yajl_version());
    return 0;
}
