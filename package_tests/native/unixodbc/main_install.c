/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for unixodbc: install; calls SQLValidDSN(), SQLSetConfigMode(), SQLGetConfigMode().
 */

#include <odbcinst.h>
#include <assert.h>
#include <stdio.h>

/* odbcinst.h does not define TRUE/FALSE; BOOL is int and the installer calls return 0 or 1. */
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

int main(void) {
    /* SQLValidDSN: reject reserved character. */
    BOOL bad = SQLValidDSN("Demo[Bad]Name");
    assert(bad == FALSE);

    /* SQLValidDSN: accept a plain name. If this were stub-returning-FALSE,
     * the first assert would have passed accidentally; this catches it. */
    BOOL good = SQLValidDSN("DemoGoodName");
    assert(good == TRUE);

    BOOL set_rc = SQLSetConfigMode(ODBC_USER_DSN);
    assert(set_rc == TRUE);

    UWORD mode_out = (UWORD)-1;
    BOOL get_rc = SQLGetConfigMode(&mode_out);
    assert(get_rc == TRUE);
    assert(mode_out == ODBC_USER_DSN);

    /* Reset to the default ODBC_BOTH_DSN to avoid leaking state if something
     * downstream re-enters this process (defensive — not strictly required
     * for a one-shot test binary, but good hygiene). */
    set_rc = SQLSetConfigMode(ODBC_BOTH_DSN);
    assert(set_rc == TRUE);

    printf("unixodbc installer-API test OK (mode round-trip = %u)\n",
           (unsigned)mode_out);
    return 0;
}
