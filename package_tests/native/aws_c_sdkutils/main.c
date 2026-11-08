/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_sdkutils: core API; calls aws_default_allocator(), aws_sdkutils_library_init(), aws_byte_buf_from_array(), aws_profile_collection_new_from_buffer().
 */

#include <aws/sdkutils/sdkutils.h>
#include <aws/sdkutils/aws_profile.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <aws/common/string.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_sdkutils_library_init(alloc);

    /* Minimal credentials-file payload: one profile, one property. */
    static const char ini[] =
        "[default]\n"
        "aws_access_key_id = AKIADEMO\n";
    struct aws_byte_buf buf = aws_byte_buf_from_array((const uint8_t *)ini, sizeof(ini) - 1);

    struct aws_profile_collection *coll =
        aws_profile_collection_new_from_buffer(alloc, &buf, AWS_PST_CREDENTIALS);
    assert(coll != NULL);

    struct aws_string *profile_name = aws_string_new_from_c_str(alloc, "default");
    struct aws_string *prop_name = aws_string_new_from_c_str(alloc, "aws_access_key_id");

    const struct aws_profile *profile = aws_profile_collection_get_profile(coll, profile_name);
    assert(profile != NULL);

    const struct aws_profile_property *prop = aws_profile_get_property(profile, prop_name);
    assert(prop != NULL);

    const struct aws_string *value = aws_profile_property_get_value(prop);
    assert(value != NULL);
    assert(value->len == 8);
    assert(memcmp(aws_string_bytes(value), "AKIADEMO", 8) == 0);

    aws_string_destroy(profile_name);
    aws_string_destroy(prop_name);
    aws_profile_collection_release(coll);

    aws_sdkutils_library_clean_up();

    printf("aws_c_sdkutils test OK: parsed default.aws_access_key_id=AKIADEMO\n");
    return 0;
}
