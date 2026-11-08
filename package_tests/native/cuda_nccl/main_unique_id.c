/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cuda_nccl: unique id; calls ncclGetUniqueId().
 */

#include <stdio.h>
#include <string.h>
#include <nccl.h>

int main(void) {
    ncclUniqueId id;
    memset(&id, 0, sizeof(id));
    ncclResult_t result = ncclGetUniqueId(&id);
    if (result != ncclSuccess) {
        fprintf(stderr, "ncclGetUniqueId failed: %d\n", (int)result);
        return 1;
    }
    /* A real id is never all-zero. */
    const char *bytes = (const char *)&id;
    for (size_t i = 0; i < sizeof(id); i++) {
        if (bytes[i] != 0) { printf("unique id OK\n"); return 0; }
    }
    fprintf(stderr, "ncclGetUniqueId returned an all-zero id\n");
    return 1;
}
