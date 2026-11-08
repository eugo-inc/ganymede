/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_s3: log subjects; calls aws_log_subject_name(), aws_default_allocator(), aws_s3_library_init(), aws_s3_library_clean_up().
 */

#include <aws/s3/s3.h>
#include <aws/common/allocator.h>
#include <aws/common/logging.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void check_s3_subject(aws_log_subject_t subject) {
    const char *name = aws_log_subject_name(subject);
    assert(name != NULL);
    /* "Unknown" is the documented fallback when the subject id is not
     * registered. Any real registration returns a distinct subject name. */
    assert(strcmp(name, "Unknown") != 0);
    assert(strlen(name) > 0);
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_s3_library_init(alloc);

    check_s3_subject(AWS_LS_S3_GENERAL);
    check_s3_subject(AWS_LS_S3_CLIENT);
    check_s3_subject(AWS_LS_S3_CLIENT_STATS);
    check_s3_subject(AWS_LS_S3_REQUEST);
    check_s3_subject(AWS_LS_S3_META_REQUEST);
    check_s3_subject(AWS_LS_S3_ENDPOINT);

    const char *client_name = aws_log_subject_name(AWS_LS_S3_CLIENT);

    aws_s3_library_clean_up();

    printf("aws_c_s3 log_subjects test OK: AWS_LS_S3_CLIENT=\"%s\"\n", client_name);
    return 0;
}
