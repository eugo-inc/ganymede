/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freexl: format open; calls freexl_open_xlsx(), freexl_open_ods(), freexl_open_info().
 */

#include <freexl.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    const void *h_xlsx = NULL;
    int rc_xlsx = freexl_open_xlsx(
        "/nonexistent/path/__demo_freexl_smoke.xlsx", &h_xlsx);
    assert(rc_xlsx != FREEXL_OK);

    const void *h_ods = NULL;
    int rc_ods = freexl_open_ods(
        "/nonexistent/path/__demo_freexl_smoke.ods", &h_ods);
    assert(rc_ods != FREEXL_OK);

    const void *h_info = NULL;
    int rc_info = freexl_open_info(
        "/nonexistent/path/__demo_freexl_smoke.xls", &h_info);
    assert(rc_info != FREEXL_OK);

    printf("freexl format-open test OK (xlsx=%d ods=%d info=%d)\n",
           rc_xlsx, rc_ods, rc_info);
    return 0;
}
