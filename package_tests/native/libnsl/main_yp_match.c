/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libnsl: yp match; calls yp_match(), yperr_string(), ypprot_err().
 */

#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char *outval = NULL;
    int outvallen = 0;
    const char *domain = "demo-libnsl-test-nonexistent-domain";
    const char *map = "passwd.byname";
    const char *key = "root";

    int rc = yp_match(domain, map, key, (int)strlen(key),
                      &outval, &outvallen);
    assert(rc != 0);
    assert(outval == NULL);

    /* yperr_string must stringify whatever rc came back. */
    const char *err_msg = yperr_string(rc);
    assert(err_msg != NULL);
    assert(strlen(err_msg) > 0);

    /* ypprot_err: YP_NOMAP (a wire-protocol "no such map" code from yp_prot.h)
     * must translate to a non-zero YPERR_* client error code that yperr_string
     * can in turn stringify. Proves the wire-to-client translation table is
     * wired up + that yp_prot.h symbols co-link with ypclnt.h symbols. */
    int client_err = ypprot_err(YP_NOMAP);
    assert(client_err != 0);
    const char *client_msg = yperr_string(client_err);
    assert(client_msg != NULL);
    assert(strlen(client_msg) > 0);

    printf("libnsl yp_match test OK: rc=%d (%s); ypprot_err(YP_NOMAP)=%d (%s)\n",
           rc, err_msg, client_err, client_msg);
    return 0;
}
