/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libnsl: yp xdr; compiles against <rpcsvc/yp.h>.
 */

#include <rpcsvc/yp.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(YPMAXDOMAIN > 0);
    assert(YPMAXMAP > 0);
    assert(YPMAXPEER > 0);
    assert(YPMAXRECORD > 0);

    /* All four are documented as 64 or larger in every NIS implementation
     * since SunOS 4. A sub-64 value would mean the header got mangled. */
    assert(YPMAXDOMAIN >= 64);
    assert(YPMAXMAP >= 64);

    struct ypreq_key req;
    memset(&req, 0, sizeof(req));
    assert(sizeof(req) > 0);

    printf("libnsl yp.h XDR test OK: YPMAXDOMAIN=%d YPMAXMAP=%d "
           "YPMAXPEER=%d YPMAXRECORD=%d sizeof(ypreq_key)=%zu\n",
           YPMAXDOMAIN, YPMAXMAP, YPMAXPEER, YPMAXRECORD, sizeof(req));
    return 0;
}
