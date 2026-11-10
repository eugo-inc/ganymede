/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for kwiml: core API; compiles against <kwiml/abi.h>, <kwiml/int.h>, <kwiml/version.h>.
 */

#include <kwiml/abi.h>
#include <kwiml/int.h>
#include <kwiml/version.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    KWIML_INT_int64_t value = KWIML_INT_INT64_C(-1234567890123);
    char text[64];
    snprintf(text, sizeof(text), "%" KWIML_INT_PRId64, value);
    if (sizeof(value) != 8 || strcmp(text, "-1234567890123") != 0) {
        fprintf(stderr, "int64: size %zu, formatted %s\n", sizeof(value), text);
        return 1;
    }
    const unsigned int probe = 1;
    const int is_little = *(const unsigned char *)&probe == 1;
#if KWIML_ABI_ENDIAN_ID == KWIML_ABI_ENDIAN_ID_LITTLE
    const int kwiml_little = 1;
#else
    const int kwiml_little = 0;
#endif
    if (is_little != kwiml_little) {
        fprintf(stderr, "byte order: machine little=%d, KWIML little=%d\n", is_little, kwiml_little);
        return 1;
    }
    printf("kwiml %s OK\n", KWIML_VERSION_STRING);
    return 0;
}
