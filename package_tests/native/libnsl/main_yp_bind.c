/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libnsl: yp bind; calls yp_bind(), yperr_string(), yp_unbind().
 */

#include <rpcsvc/ypclnt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int rc = yp_bind("demo-libnsl-test-nonexistent-domain");
    /* Whatever it returns, yperr_string must handle it without NULL. */
    const char *bind_msg = yperr_string(rc);
    assert(bind_msg != NULL);
    assert(strlen(bind_msg) > 0);

    yp_unbind("demo-libnsl-test-nonexistent-domain");

    printf("libnsl yp_bind test OK: rc=%d (%s)\n", rc, bind_msg);
    return 0;
}
