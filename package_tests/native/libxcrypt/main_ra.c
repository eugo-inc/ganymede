/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libxcrypt: ra; calls crypt_gensalt_ra(), crypt_ra().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <crypt.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Step 1: crypt_gensalt_ra — heap-allocating salt generation.
     * Passing rbytes=NULL asks libxcrypt to read /dev/urandom itself. */
    char *setting = crypt_gensalt_ra("$y$", 0, NULL, 0);
    assert(setting != NULL);
    assert(strncmp(setting, "$y$", 3) == 0);
    assert(strlen(setting) > 3);

    /* Step 2: crypt_ra — heap-allocating hash. The buffer pointer + size
     * are in/out parameters: passing (NULL, 0) asks libxcrypt to malloc()
     * a fresh buffer; on success the caller owns it. */
    void *data = NULL;
    int data_size = 0;
    char *hash = crypt_ra("hello", setting, &data, &data_size);
    assert(hash != NULL);
    assert(data != NULL);
    assert(data_size > 0);
    /* hash points INTO the allocated buffer (specifically the `output`
     * field of the underlying struct crypt_data). */
    assert((char *)data <= hash &&
           hash < (char *)data + (size_t)data_size);
    assert(strncmp(hash, "$y$", 3) == 0);

    /* Save the produced hash before reusing the buffer. */
    char produced[CRYPT_OUTPUT_SIZE];
    strncpy(produced, hash, sizeof(produced) - 1);
    produced[sizeof(produced) - 1] = '\0';

    char *again = crypt_ra("hello", produced, &data, &data_size);
    assert(again != NULL);
    assert(strcmp(again, produced) == 0);

    /* Negative path: wrong password must NOT reproduce the hash. */
    char *bad = crypt_ra("world", produced, &data, &data_size);
    assert(bad != NULL);
    assert(strcmp(bad, produced) != 0);

    /* Step 4: own + free the heap allocations — proves the ownership
     * transfer to the caller (would leak under valgrind otherwise). */
    free(setting);
    free(data);

    printf("libxcrypt ra test OK: %s\n", produced);
    return 0;
}
