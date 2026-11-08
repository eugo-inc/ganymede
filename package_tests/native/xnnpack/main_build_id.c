/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for xnnpack: build id; calls xnn_experimental_get_build_identifier_data(), xnn_experimental_get_build_identifier_size(), xnn_experimental_check_build_identifier().
 */

#include <xnnpack.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    const void* id_data = xnn_experimental_get_build_identifier_data();
    assert(id_data != NULL);

    size_t id_size = xnn_experimental_get_build_identifier_size();
    assert(id_size > 0);

    /* Self-identification must succeed: feeding the library its own identifier
     * back is the canonical "do these halves match?" check. */
    bool matches = xnn_experimental_check_build_identifier(id_data, id_size);
    assert(matches);

    /* Mismatched size must NOT spuriously match. */
    bool wrong_size_matches =
        xnn_experimental_check_build_identifier(id_data, id_size - 1);
    assert(!wrong_size_matches);

    printf("xnnpack build_id test OK (size=%zu)\n", id_size);
    return 0;
}
