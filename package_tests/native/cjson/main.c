/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cjson: core API; calls cJSON_Version(), cJSON_Parse(), cJSON_IsString(), cJSON_GetObjectItemCaseSensitive().
 */

#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Always evaluated, even with NDEBUG. */
#define CHECK(condition)                                                                 \
    do {                                                                                 \
        if (!(condition)) {                                                              \
            fprintf(stderr, "CJSON: FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
            return 1;                                                                    \
        }                                                                                \
    } while (0)

int main(void) {
    char expected_version[32];
    snprintf(expected_version, sizeof(expected_version), "%d.%d.%d", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR,
             CJSON_VERSION_PATCH);
    CHECK(strcmp(cJSON_Version(), expected_version) == 0);

    cJSON *doc = cJSON_Parse("{\"name\":\"demo\",\"pi\":3.25,\"ok\":false,\"none\":null,\"list\":[\"a\",\"b\"]}");
    CHECK(doc != NULL);
    CHECK(cJSON_IsString(cJSON_GetObjectItemCaseSensitive(doc, "name")));
    CHECK(strcmp(cJSON_GetObjectItemCaseSensitive(doc, "name")->valuestring, "demo") == 0);
    CHECK(cJSON_GetObjectItemCaseSensitive(doc, "pi")->valuedouble == 3.25);
    CHECK(cJSON_IsFalse(cJSON_GetObjectItemCaseSensitive(doc, "ok")));
    CHECK(cJSON_IsNull(cJSON_GetObjectItemCaseSensitive(doc, "none")));
    CHECK(cJSON_GetArraySize(cJSON_GetObjectItemCaseSensitive(doc, "list")) == 2);

    cJSON *built = cJSON_CreateObject();
    CHECK(built != NULL);
    CHECK(cJSON_AddNumberToObject(built, "n", 7) != NULL);
    CHECK(cJSON_AddStringToObject(built, "s", "x\"y") != NULL);
    char *printed = cJSON_PrintUnformatted(built);
    CHECK(printed != NULL);
    CHECK(strcmp(printed, "{\"n\":7,\"s\":\"x\\\"y\"}") == 0);
    cJSON *again = cJSON_Parse(printed);
    CHECK(again != NULL);
    CHECK(cJSON_Compare(built, again, 1));

    CHECK(cJSON_Parse("{\"unterminated\": [1, 2") == NULL);

    cJSON_free(printed);
    cJSON_Delete(again);
    cJSON_Delete(built);
    cJSON_Delete(doc);
    printf("CJSON: PASS %s parse/print round trip\n", cJSON_Version());
    return 0;
}
