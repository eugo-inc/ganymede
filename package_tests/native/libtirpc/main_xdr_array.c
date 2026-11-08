/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libtirpc: xdr array; calls xdrmem_create(), xdr_array(), xdr_destroy().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/xdr.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char buf[256];
    XDR xdr_enc;
    XDR xdr_dec;

    /* Encode a variable-length array of 5 ints. */
    xdrmem_create(&xdr_enc, buf, sizeof(buf), XDR_ENCODE);
    int in_arr[] = {11, 22, 33, 44, 55};
    u_int in_len = 5;
    int *in_ptr = in_arr;
    assert(xdr_array(&xdr_enc,
                     (char **)&in_ptr,
                     &in_len,
                     16,
                     sizeof(int),
                     (xdrproc_t)xdr_int) == TRUE);
    xdr_destroy(&xdr_enc);

    xdrmem_create(&xdr_dec, buf, sizeof(buf), XDR_DECODE);
    int *out_ptr = NULL;
    u_int out_len = 0;
    assert(xdr_array(&xdr_dec,
                     (char **)&out_ptr,
                     &out_len,
                     16,
                     sizeof(int),
                     (xdrproc_t)xdr_int) == TRUE);
    assert(out_len == 5);
    assert(out_ptr != NULL);
    for (u_int i = 0; i < out_len; i++) {
        assert(out_ptr[i] == in_arr[i]);
    }
    free(out_ptr);
    xdr_destroy(&xdr_dec);

    printf("libtirpc xdr_array test OK\n");
    return 0;
}
