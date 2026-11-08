/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_compression: error codes; calls aws_default_allocator(), aws_compression_library_init(), aws_error_str(), aws_error_name().
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

    aws_compression_library_init(alloc);

    const int base = AWS_ERROR_COMPRESSION_UNKNOWN_SYMBOL;

    int known_registered = 0;
    for (int offset = 0; offset < 16; ++offset) {
        int code = base + offset;
        const char *msg = aws_error_str(code);
        const char *name = aws_error_name(code);
        assert(msg != NULL);
        assert(name != NULL);
        assert(msg[0] != '\0');
        assert(name[0] != '\0');

        /* A registered code's name is the macro identifier (starts with "AWS_ERROR_").
         * Unregistered slot's name from aws_c_common is "Unknown Error Code". */
        if (strncmp(name, "AWS_ERROR_", 10) == 0) {
            ++known_registered;
        }
    }

    /* At minimum the documented sentinel must be a real registered code. */
    assert(known_registered >= 1);

    const char *sentinel_name = aws_error_name(AWS_ERROR_COMPRESSION_UNKNOWN_SYMBOL);
    assert(strstr(sentinel_name, "UNKNOWN_SYMBOL") != NULL);

    aws_compression_library_clean_up();

    printf("aws_c_compression error_codes OK: %d/%d registered in [0x%X..0x%X]\n",
           known_registered, 16, base, base + 15);
    return 0;
}
