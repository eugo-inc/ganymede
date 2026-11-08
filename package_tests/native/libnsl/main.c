/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libnsl: core API; calls yperr_string(), yp_get_default_domain().
 */

#include <rpcsvc/ypclnt.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* yperr_string must return a non-NULL human string for the success code. */
    const char *ok_msg = yperr_string(YPERR_SUCCESS);
    assert(ok_msg != NULL);
    assert(strlen(ok_msg) > 0);

    char *domain = NULL;
    int rc = yp_get_default_domain(&domain);
    const char *rc_msg = yperr_string(rc);
    assert(rc_msg != NULL);

    printf("libnsl test OK: yp_get_default_domain rc=%d (%s)\n", rc, rc_msg);
    return 0;
}
