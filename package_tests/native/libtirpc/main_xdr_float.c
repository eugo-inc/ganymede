/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtirpc: xdr float; calls xdrmem_create(), xdr_double(), xdr_bool(), xdr_destroy().
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

    /* Encode: double + uint64 + bool. */
    xdrmem_create(&xdr_enc, buf, sizeof(buf), XDR_ENCODE);
    double in_d = 3.14159265358979323846;
    uint64_t in_u64 = 0x0123456789ABCDEFULL;
    bool_t in_b = TRUE;
    assert(xdr_double(&xdr_enc, &in_d) == TRUE);
    assert(xdr_u_int64_t(&xdr_enc, &in_u64) == TRUE);
    assert(xdr_bool(&xdr_enc, &in_b) == TRUE);
    xdr_destroy(&xdr_enc);

    /* Decode: pull the same values back out, bitwise-identical. */
    xdrmem_create(&xdr_dec, buf, sizeof(buf), XDR_DECODE);
    double out_d = 0.0;
    uint64_t out_u64 = 0;
    bool_t out_b = FALSE;
    assert(xdr_double(&xdr_dec, &out_d) == TRUE);
    assert(xdr_u_int64_t(&xdr_dec, &out_u64) == TRUE);
    assert(xdr_bool(&xdr_dec, &out_b) == TRUE);

    assert(memcmp(&in_d, &out_d, sizeof(double)) == 0);
    assert(out_u64 == 0x0123456789ABCDEFULL);
    assert(out_b == TRUE);
    xdr_destroy(&xdr_dec);

    printf("libtirpc xdr_float test OK\n");
    return 0;
}
