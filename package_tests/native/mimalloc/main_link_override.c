/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for mimalloc: link override; calls mi_version(), posix_memalign(), mi_is_in_heap_region(), mi_option_get().
 */

#include <mimalloc.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int failures = 0;
    if (mi_version() != MI_MALLOC_VERSION) {
        fprintf(stderr, "FAIL mi_version() %d != header %d\n", mi_version(), MI_MALLOC_VERSION);
        failures++;
    }
    void *a = malloc(1000);
    void *b = calloc(100, 16);
    void *c = realloc(malloc(64), 1 << 20);
    void *d = NULL;
    if (posix_memalign(&d, 4096, 12345) != 0) {
        fprintf(stderr, "FAIL posix_memalign\n");
        failures++;
    }
    void *ptrs[4] = {a, b, c, d};
    for (int i = 0; i < 4; i++) {
        if (ptrs[i] == NULL || !mi_is_in_heap_region(ptrs[i])) {
            fprintf(stderr, "FAIL allocation %d (%p) is not in mimalloc's heap: the malloc override is missing\n", i, ptrs[i]);
            failures++;
        }
    }
    if (((unsigned long)d & 4095UL) != 0) {
        fprintf(stderr, "FAIL posix_memalign returned %p, not 4096-aligned\n", d);
        failures++;
    }
    if (getenv("MIMALLOC_ALLOW_THP") == NULL && mi_option_get(mi_option_allow_thp) != 2) {
        fprintf(stderr, "FAIL allow_thp default %ld, expected 2 (MI_ALLOW_THP=FULL)\n", mi_option_get(mi_option_allow_thp));
        failures++;
    }
    for (int i = 0; i < 4; i++) {
        free(ptrs[i]);
    }
    if (failures == 0) {
        printf("OK mimalloc %d overrides malloc/calloc/realloc/posix_memalign; allow_thp default 2\n", mi_version());
    }
    return failures == 0 ? 0 : 1;
}
