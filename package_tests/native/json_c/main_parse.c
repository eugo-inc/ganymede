/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for json_c: parse; calls json_tokener_parse(), json_object_is_type(), json_object_object_get_ex(), json_object_get_string().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json-c/json.h>
#include <json-c/json_tokener.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *src =
        "{\"name\":\"demo\",\"count\":42,\"ok\":true,\"ratio\":1.5,\"nil\":null}";

    struct json_object *root = json_tokener_parse(src);
    assert(root != NULL);
    assert(json_object_is_type(root, json_type_object));

    struct json_object *name = NULL;
    assert(json_object_object_get_ex(root, "name", &name));
    assert(name != NULL);
    assert(json_object_is_type(name, json_type_string));
    assert(strcmp(json_object_get_string(name), "demo") == 0);

    struct json_object *count = NULL;
    assert(json_object_object_get_ex(root, "count", &count));
    assert(json_object_is_type(count, json_type_int));
    assert(json_object_get_int(count) == 42);

    struct json_object *ok = NULL;
    assert(json_object_object_get_ex(root, "ok", &ok));
    assert(json_object_is_type(ok, json_type_boolean));
    assert(json_object_get_boolean(ok) == 1);

    struct json_object *ratio = NULL;
    assert(json_object_object_get_ex(root, "ratio", &ratio));
    assert(json_object_is_type(ratio, json_type_double));
    /* exact value is representable; tolerate FP slop anyway */
    double r = json_object_get_double(ratio);
    assert(r > 1.49 && r < 1.51);

    struct json_object *nil = NULL;
    assert(json_object_object_get_ex(root, "nil", &nil));
    assert(nil == NULL || json_object_is_type(nil, json_type_null));

    /* Negative test: malformed JSON must fail to parse. */
    struct json_object *bad = json_tokener_parse("{not json");
    assert(bad == NULL);

    json_object_put(root);
    printf("json_c parse OK\n");
    return 0;
}
