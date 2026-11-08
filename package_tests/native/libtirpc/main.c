/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libtirpc: core API; calls xdrmem_create(), xdr_string(), xdr_destroy().
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

    /* Encode: int32 + string into the buffer. */
    xdrmem_create(&xdr_enc, buf, sizeof(buf), XDR_ENCODE);
    int32_t in_int = 0x12345678;
    char *in_str = (char *)"hello-tirpc";
    assert(xdr_int32_t(&xdr_enc, &in_int) == TRUE);
    assert(xdr_string(&xdr_enc, &in_str, 64) == TRUE);
    xdr_destroy(&xdr_enc);

    /* Decode: pull the same values back out. */
    xdrmem_create(&xdr_dec, buf, sizeof(buf), XDR_DECODE);
    int32_t out_int = 0;
    char *out_str = NULL;
    assert(xdr_int32_t(&xdr_dec, &out_int) == TRUE);
    assert(xdr_string(&xdr_dec, &out_str, 64) == TRUE);
    assert(out_int == 0x12345678);
    assert(out_str != NULL);
    assert(strcmp(out_str, "hello-tirpc") == 0);
    free(out_str);
    xdr_destroy(&xdr_dec);

    printf("libtirpc test OK\n");
    return 0;
}
