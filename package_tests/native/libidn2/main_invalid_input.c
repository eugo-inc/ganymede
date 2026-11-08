/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libidn2: invalid input; calls idn2_lookup_u8(), idn2_strerror(), idn2_free(), idn2_to_unicode_8z8z().
 */

#include <idn2.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    {
        uint8_t *out = NULL;
        int rc = idn2_lookup_u8(
            (const uint8_t *)"-leadinghyphen.example",
            &out,
            IDN2_NONTRANSITIONAL
        );
        assert(rc != IDN2_OK);
        assert(rc == IDN2_HYPHEN_STARTEND);

        const char *msg = idn2_strerror(rc);
        assert(msg != NULL);
        assert(strlen(msg) > 0);
        assert(strcmp(msg, idn2_strerror(IDN2_OK)) != 0);

        if (out != NULL) {
            idn2_free(out);
        }
    }

    {
        uint8_t *out = NULL;
        int rc = idn2_lookup_u8(
            (const uint8_t *)"ab--cd.example",
            &out,
            IDN2_NONTRANSITIONAL
        );
        assert(rc != IDN2_OK);
        assert(rc == IDN2_2HYPHEN);

        if (out != NULL) {
            idn2_free(out);
        }
    }

    {
        char *out = NULL;
        /* "xn--" followed by characters that are not valid Punycode-base-36. */
        int rc = idn2_to_unicode_8z8z("xn--!!!invalid!!!", &out, 0);
        assert(rc != IDN2_OK);
        assert(rc == IDN2_PUNYCODE_BAD_INPUT);

        const char *msg = idn2_strerror(rc);
        assert(msg != NULL);
        assert(strlen(msg) > 0);

        if (out != NULL) {
            idn2_free(out);
        }
    }

    printf("libidn2 invalid_input OK: all 3 negative probes rejected\n");
    return 0;
}
