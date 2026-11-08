/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for utf8proc: core API; calls utf8proc_version(), utf8proc_iterate(), utf8proc_NFC().
 */

#include <utf8proc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* Version string must be non-NULL and non-empty. */
    const char *v = utf8proc_version();
    assert(v != NULL);
    assert(strlen(v) > 0);

    /* Decode "é" (U+00E9, encoded as 0xC3 0xA9 in UTF-8). */
    const utf8proc_uint8_t e_acute[] = {0xC3, 0xA9, 0x00};
    utf8proc_int32_t cp = 0;
    utf8proc_ssize_t consumed = utf8proc_iterate(e_acute, 2, &cp);
    assert(consumed == 2);
    assert(cp == 0x00E9);

    /* NFC-normalize the decomposed form "e" + U+0301 (COMBINING ACUTE ACCENT, UTF-8: 0xCC 0x81).
     * Expected result: the single precomposed codepoint U+00E9 ("é") = 0xC3 0xA9. */
    const utf8proc_uint8_t decomposed[] = {'e', 0xCC, 0x81, 0x00};
    utf8proc_uint8_t *nfc = utf8proc_NFC(decomposed);
    assert(nfc != NULL);
    assert(strcmp((const char *)nfc, (const char *)e_acute) == 0);
    free(nfc);

    printf("utf8proc test OK: %s\n", v);
    return 0;
}
