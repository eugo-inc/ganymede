/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_common: core API; calls aws_default_allocator(), aws_common_library_init(), aws_string_new_from_c_str(), aws_string_bytes().
 */

#include <aws/common/common.h>
#include <aws/common/allocator.h>
#include <aws/common/string.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_common_library_init(alloc);

    /* Exercise the string subsystem — allocates via the default allocator. */
    struct aws_string *s = aws_string_new_from_c_str(alloc, "demo");
    assert(s != NULL);
    assert(s->len == 4);
    assert(memcmp(aws_string_bytes(s), "demo", 4) == 0);
    aws_string_destroy(s);

    /* aws_error_str must return a non-NULL human-readable string for the
     * sentinel "out of memory" code — catches a missing error-table init. */
    const char *err = aws_error_str(AWS_ERROR_OOM);
    assert(err != NULL);
    assert(strlen(err) > 0);

    aws_common_library_clean_up();

    printf("aws_c_common test OK: error_str(OOM)=\"%s\"\n", err);
    return 0;
}
