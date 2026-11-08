/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_sdkutils: endpoints; calls aws_default_allocator(), aws_sdkutils_library_init(), aws_endpoints_get_supported_ruleset_version(), aws_byte_cursor_from_array().
 */

#include <aws/sdkutils/sdkutils.h>
#include <aws/sdkutils/endpoints_rule_engine.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/hash_table.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static const char k_minimal_ruleset_json[] =
    "{"
    "\"version\":\"1.0\","
    "\"parameters\":{},"
    "\"rules\":[]"
    "}";

/* Malformed: missing the required "rules" array. */
static const char k_bad_ruleset_json[] =
    "{"
    "\"version\":\"1.0\","
    "\"parameters\":{}"
    "}";

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_sdkutils_library_init(alloc);

    /* --- Supported-ruleset-version cursor is reported and non-empty. --- */
    struct aws_byte_cursor supported = aws_endpoints_get_supported_ruleset_version();
    assert(supported.ptr != NULL);
    assert(supported.len > 0);

    /* --- Positive: schema-valid JSON parses into a ruleset handle. --- */
    struct aws_byte_cursor body = aws_byte_cursor_from_array(
        (const uint8_t *)k_minimal_ruleset_json, sizeof(k_minimal_ruleset_json) - 1);
    struct aws_endpoints_ruleset *ruleset = aws_endpoints_ruleset_new_from_string(alloc, body);
    assert(ruleset != NULL);

    struct aws_byte_cursor got_version = aws_endpoints_ruleset_get_version(ruleset);
    assert(got_version.len == 3);
    assert(memcmp(got_version.ptr, "1.0", 3) == 0);

    /* --- Introspection: parameters hashtable accessor never returns NULL,
     *     and an empty `parameters` object yields a 0-element table. --- */
    const struct aws_hash_table *params = aws_endpoints_ruleset_get_parameters(ruleset);
    assert(params != NULL);
    assert(aws_hash_table_get_entry_count(params) == 0);

    /* --- Ref-count lifecycle: acquire returns the same handle,
     *     release returns NULL on both the duplicate ref and the original. --- */
    struct aws_endpoints_ruleset *same = aws_endpoints_ruleset_acquire(ruleset);
    assert(same == ruleset);
    struct aws_endpoints_ruleset *after_first_release = aws_endpoints_ruleset_release(ruleset);
    assert(after_first_release == NULL);
    struct aws_endpoints_ruleset *after_final_release = aws_endpoints_ruleset_release(ruleset);
    assert(after_final_release == NULL);

    /* --- Negative: a ruleset missing the required "rules" field is rejected. --- */
    struct aws_byte_cursor bad_body = aws_byte_cursor_from_array(
        (const uint8_t *)k_bad_ruleset_json, sizeof(k_bad_ruleset_json) - 1);
    struct aws_endpoints_ruleset *bad = aws_endpoints_ruleset_new_from_string(alloc, bad_body);
    assert(bad == NULL);

    aws_sdkutils_library_clean_up();

    printf("aws_c_sdkutils endpoints test OK: parsed + introspected + ref-cycled + rejected-malformed\n");
    return 0;
}
