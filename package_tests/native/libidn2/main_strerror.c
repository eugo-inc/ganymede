/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libidn2: strerror; calls idn2_check_version(), idn2_strerror(), idn2_strerror_name().
 */

#include <idn2.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/*
 * Aspect: error reporting + version interrogation.
 *
 * - idn2_check_version(NULL) must return a non-NULL version string
 *   (compatibility check against the runtime libidn2).
 * - idn2_check_version("99.99.99") must return NULL (no library is
 *   newer than this absurd version).
 * - idn2_strerror() / idn2_strerror_name() must yield non-empty
 *   distinct strings for IDN2_OK vs a real error code.
 */
int main(void) {
    /* Runtime version interrogation. */
    const char *ver = idn2_check_version(NULL);
    assert(ver != NULL);
    assert(strlen(ver) > 0);

    /* This must fail — no realistic libidn2 release is >= 99.99.99. */
    const char *too_new = idn2_check_version("99.99.99");
    assert(too_new == NULL);

    /* Error-string lookup: distinct strings, neither empty. */
    const char *ok_msg = idn2_strerror(IDN2_OK);
    const char *bad_msg = idn2_strerror(IDN2_PUNYCODE_BAD_INPUT);
    assert(ok_msg != NULL && strlen(ok_msg) > 0);
    assert(bad_msg != NULL && strlen(bad_msg) > 0);
    assert(strcmp(ok_msg, bad_msg) != 0);

    /* Symbolic-name lookup: must yield the spelled-out constant name. */
    const char *ok_name = idn2_strerror_name(IDN2_OK);
    assert(ok_name != NULL);
    assert(strcmp(ok_name, "IDN2_OK") == 0);

    printf("libidn2 strerror OK: version=%s\n", ver);
    return 0;
}
