/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_compression: lifecycle; calls aws_default_allocator(), aws_compression_library_init(), aws_error_str(), aws_compression_library_clean_up().
 */

#include <aws/compression/compression.h>
#include <aws/common/allocator.h>
#include <aws/common/error.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    const int CYCLES = 4;
    for (int i = 0; i < CYCLES; ++i) {
        aws_compression_library_init(alloc);

        /* Error table MUST be available after EVERY init, not just the first. */
        const char *err = aws_error_str(AWS_ERROR_COMPRESSION_UNKNOWN_SYMBOL);
        assert(err != NULL);
        assert(err[0] != '\0');
        assert(strstr(err, "unknown symbol") != NULL);

        /* Allocator must still be the same object — library_init must not replace it. */
        assert(aws_default_allocator() == alloc);

        aws_compression_library_clean_up();
    }

    /* After final cleanup, the default allocator must still be alive (singleton). */
    assert(aws_default_allocator() == alloc);

    printf("aws_c_compression lifecycle OK: %d init/clean_up cycles\n", CYCLES);
    return 0;
}
