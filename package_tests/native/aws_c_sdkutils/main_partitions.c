/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_sdkutils: partitions; calls aws_default_allocator(), aws_sdkutils_library_init(), aws_partitions_get_supported_version(), aws_byte_cursor_from_array().
 */

#include <aws/sdkutils/sdkutils.h>
#include <aws/sdkutils/partitions.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static const char k_partitions_json[] =
    "{"
    "\"version\":\"1.1\","
    "\"partitions\":["
    "{"
    "\"id\":\"aws\","
    "\"regionRegex\":\"^(us|eu|ap)\\\\-\\\\w+\\\\-\\\\d+$\","
    "\"regions\":{\"us-east-1\":{}},"
    "\"outputs\":{"
    "\"name\":\"aws\","
    "\"dnsSuffix\":\"amazonaws.com\","
    "\"dualStackDnsSuffix\":\"api.aws\","
    "\"supportsFIPS\":true,"
    "\"supportsDualStack\":true"
    "}"
    "}"
    "]"
    "}";

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_sdkutils_library_init(alloc);

    /* --- Supported-version cursor is reported and non-empty. --- */
    struct aws_byte_cursor version = aws_partitions_get_supported_version();
    assert(version.ptr != NULL);
    assert(version.len > 0);

    /* --- Positive: schema-valid JSON parses into a config handle. --- */
    struct aws_byte_cursor body = aws_byte_cursor_from_array(
        (const uint8_t *)k_partitions_json, sizeof(k_partitions_json) - 1);
    struct aws_partitions_config *cfg = aws_partitions_config_new_from_string(alloc, body);
    assert(cfg != NULL);

    /* --- Ref-count lifecycle: acquire returns the same handle, release NULL. --- */
    struct aws_partitions_config *same = aws_partitions_config_acquire(cfg);
    assert(same == cfg);
    /* Drop the extra ref from acquire. */
    struct aws_partitions_config *after_first_release = aws_partitions_config_release(cfg);
    assert(after_first_release == NULL);
    /* Drop the original ref. */
    struct aws_partitions_config *after_final_release = aws_partitions_config_release(cfg);
    assert(after_final_release == NULL);

    /* --- Negative: garbage JSON is rejected. --- */
    struct aws_byte_cursor garbage = aws_byte_cursor_from_c_str("{not valid json");
    struct aws_partitions_config *bad = aws_partitions_config_new_from_string(alloc, garbage);
    assert(bad == NULL);

    aws_sdkutils_library_clean_up();

    printf("aws_c_sdkutils partitions test OK: loaded + ref-cycled + rejected-garbage\n");
    return 0;
}
