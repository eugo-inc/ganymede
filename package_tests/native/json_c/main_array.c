/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for json_c: array; calls json_object_new_array(), json_object_is_type(), json_object_array_length(), json_object_array_add().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <json-c/json.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    struct json_object *arr = json_object_new_array();
    assert(arr != NULL);
    assert(json_object_is_type(arr, json_type_array));
    assert(json_object_array_length(arr) == 0);

    /* Append 5 distinct ints. */
    for (int i = 0; i < 5; i++) {
        assert(json_object_array_add(arr, json_object_new_int(i * 10)) == 0);
    }
    assert(json_object_array_length(arr) == 5);

    /* Indexed access. */
    for (size_t i = 0; i < 5; i++) {
        struct json_object *elt = json_object_array_get_idx(arr, i);
        assert(elt != NULL);
        assert(json_object_is_type(elt, json_type_int));
        assert(json_object_get_int(elt) == (int)i * 10);
    }

    /* Iteration via length + index — visit every element once. */
    int sum = 0;
    int seen = 0;
    size_t n = json_object_array_length(arr);
    for (size_t i = 0; i < n; i++) {
        struct json_object *elt = json_object_array_get_idx(arr, i);
        sum += json_object_get_int(elt);
        seen++;
    }
    assert(seen == 5);
    assert(sum == 0 + 10 + 20 + 30 + 40);

    /* Replace an element via array_put_idx. */
    assert(json_object_array_put_idx(arr, 2, json_object_new_int(999)) == 0);
    struct json_object *replaced = json_object_array_get_idx(arr, 2);
    assert(json_object_get_int(replaced) == 999);
    assert(json_object_array_length(arr) == 5);

    json_object_put(arr);
    printf("json_c array OK\n");
    return 0;
}
