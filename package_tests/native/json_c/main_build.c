/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for json_c: build; calls json_object_new_object(), json_object_object_add(), json_object_new_string(), json_object_new_int().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json-c/json.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct json_object *root = json_object_new_object();
    assert(root != NULL);

    assert(json_object_object_add(root, "s", json_object_new_string("hello")) == 0);
    assert(json_object_object_add(root, "i", json_object_new_int(7)) == 0);
    assert(json_object_object_add(root, "b", json_object_new_boolean(1)) == 0);

    struct json_object *child = json_object_new_object();
    assert(child != NULL);
    assert(json_object_object_add(child, "k", json_object_new_string("v")) == 0);
    assert(json_object_object_add(root, "child", child) == 0);

    /* Serialize. JSON_C_TO_STRING_PLAIN == 0 — compact, no whitespace. */
    const char *out = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PLAIN);
    assert(out != NULL);
    assert(strstr(out, "\"s\":\"hello\"") != NULL);
    assert(strstr(out, "\"i\":7") != NULL);
    assert(strstr(out, "\"b\":true") != NULL);
    assert(strstr(out, "\"child\":{") != NULL);
    assert(strstr(out, "\"k\":\"v\"") != NULL);

    char buf[256];
    size_t n = strlen(out);
    assert(n < sizeof(buf));
    memcpy(buf, out, n + 1);

    struct json_object *parsed = json_tokener_parse(buf);
    assert(parsed != NULL);
    assert(json_object_is_type(parsed, json_type_object));

    struct json_object *p_s = NULL;
    assert(json_object_object_get_ex(parsed, "s", &p_s));
    assert(json_object_is_type(p_s, json_type_string));
    assert(strcmp(json_object_get_string(p_s), "hello") == 0);

    struct json_object *p_i = NULL;
    assert(json_object_object_get_ex(parsed, "i", &p_i));
    assert(json_object_is_type(p_i, json_type_int));
    assert(json_object_get_int(p_i) == 7);

    struct json_object *p_b = NULL;
    assert(json_object_object_get_ex(parsed, "b", &p_b));
    assert(json_object_is_type(p_b, json_type_boolean));
    assert(json_object_get_boolean(p_b) == 1);

    struct json_object *p_child = NULL;
    assert(json_object_object_get_ex(parsed, "child", &p_child));
    assert(json_object_is_type(p_child, json_type_object));

    struct json_object *p_k = NULL;
    assert(json_object_object_get_ex(p_child, "k", &p_k));
    assert(json_object_is_type(p_k, json_type_string));
    assert(strcmp(json_object_get_string(p_k), "v") == 0);

    json_object_put(parsed);
    json_object_put(root);
    printf("json_c build OK\n");
    return 0;
}
