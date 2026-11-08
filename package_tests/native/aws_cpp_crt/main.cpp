/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_cpp_crt: core API; calls Aws::Crt::ByteCursorFromCString().
 */

#include <aws/crt/Api.h>
#include <cassert>
#include <cstring>
#include <iostream>

int main() {
    /* Constructing ApiHandle initializes every aws_c_* library this wrapper pulls in.
     * Destructor (RAII at scope exit) tears them back down. Catches link-time gaps
     * across the whole aws_c_* fanout AND a broken init order. */
    {
        Aws::Crt::ApiHandle api;

        /* Exercise a representative inline header utility that compiles down to a
         * symbol in libaws-c-common. Catches header/.so version mismatch. */
        const char *msg = "hello";
        aws_byte_cursor cursor = Aws::Crt::ByteCursorFromCString(msg);
        assert(cursor.ptr != nullptr);
        assert(cursor.len == std::strlen(msg));
    }

    std::cout << "aws_cpp_crt test OK" << std::endl;
    return 0;
}
