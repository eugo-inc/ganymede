/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for liburing: ffi; calls prep_nop().
 */

#include <liburing.h>

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

typedef void (*prep_nop_function)(struct io_uring_sqe *sqe);

int main(void) {
    void *handle = dlopen("liburing-ffi.so.2", RTLD_NOW);
    if (handle == NULL) {
        fprintf(stderr, "liburing: dlopen(liburing-ffi.so.2) failed: %s\n", dlerror());
        return 1;
    }

    prep_nop_function prep_nop = NULL;
    void *symbol = dlsym(handle, "io_uring_prep_nop");
    memcpy(&prep_nop, &symbol, sizeof prep_nop);
    if (prep_nop == NULL) {
        fprintf(stderr, "liburing: liburing-ffi.so.2 does not export io_uring_prep_nop\n");
        return 1;
    }

    struct io_uring_sqe sqe;
    memset(&sqe, 0xff, sizeof sqe);
    prep_nop(&sqe);
    if (sqe.opcode != IORING_OP_NOP) {
        fprintf(stderr, "liburing: io_uring_prep_nop left opcode %u, expected IORING_OP_NOP (%u)\n",
                (unsigned) sqe.opcode, (unsigned) IORING_OP_NOP);
        return 1;
    }

    dlclose(handle);
    printf("liburing: liburing-ffi.so.2 exports a working io_uring_prep_nop\n");
    return 0;
}
