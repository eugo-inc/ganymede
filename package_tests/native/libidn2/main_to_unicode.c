/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libidn2: to unicode; calls idn2_to_unicode_8z8z().
 */

#include <idn2.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Aspect: reverse conversion (Punycode/ACE -> Unicode UTF-8).
 *
 * "xn--nxasmq6b" is the canonical ACE form of the Greek IDN label
 * "βόλοσ" (a lowercased form of "Βόλος", the Greek city Bólos). The
 * Punycode-encoded label uses the regular (non-final) sigma 'σ'
 * (U+03C3, UTF-8 0xCF 0x83), NOT the word-final sigma 'ς' (U+03C2);
 * IDNA Punycode does not preserve final-sigma word position.
 * idn2_to_unicode_8z8z must decode the ACE label back to its UTF-8
 * byte sequence.
 *
 * Bytes: β = 0xCE 0xB2, ό = 0xCF 0x8C, λ = 0xCE 0xBB,
 *        ο = 0xCE 0xBF, σ = 0xCF 0x83
 */
int main(void) {
    char *unicode = NULL;
    int rc = idn2_to_unicode_8z8z("xn--nxasmq6b", &unicode, 0);
    assert(rc == IDN2_OK);
    assert(unicode != NULL);

    const unsigned char expected[] = {
        0xCE, 0xB2, 0xCF, 0x8C, 0xCE, 0xBB, 0xCE, 0xBF, 0xCF, 0x83, 0x00
    };
    assert(strcmp(unicode, (const char *)expected) == 0);

    printf("libidn2 to_unicode OK: decoded %zu bytes\n", strlen(unicode));
    free(unicode);
    return 0;
}
