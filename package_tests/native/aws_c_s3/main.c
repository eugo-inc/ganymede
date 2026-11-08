/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_s3: core API; calls aws_default_allocator(), aws_s3_library_init(), aws_error_str(), aws_s3_get_current_platform_info().
 */

#include <aws/s3/s3.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_s3_library_init(alloc);

    /* aws_error_str must return a non-NULL human-readable string for an
     * S3-table error — catches a missing S3 error-table registration. */
    const char *err = aws_error_str(AWS_ERROR_S3_INTERNAL_ERROR);
    assert(err != NULL);
    assert(strlen(err) > 0);

    /* Platform-info lookup must return a non-NULL record (falls back to a
     * default entry on unknown hardware). Exercises the platform subsystem. */
    const struct aws_s3_platform_info *info = aws_s3_get_current_platform_info();
    assert(info != NULL);

    aws_s3_library_clean_up();

    printf("aws_c_s3 test OK: error_str(S3_INTERNAL_ERROR)=\"%s\"\n", err);
    return 0;
}
