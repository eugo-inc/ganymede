/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libidn2: punycode; calls idn2_punycode_encode(), idn2_punycode_decode().
 */

#include <idn2.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Greek codepoints for "βόλος". */
    const uint32_t unicode_in[] = { 0x03B2, 0x03CC, 0x03BB, 0x03BF, 0x03C2 };
    const size_t unicode_in_len = sizeof(unicode_in) / sizeof(unicode_in[0]);

    /* Encode: codepoints -> Punycode. */
    char ace[64];
    size_t ace_len = sizeof(ace);  /* in/out: capacity in, actual out. */
    int rc = idn2_punycode_encode(unicode_in, unicode_in_len, ace, &ace_len);
    assert(rc == IDN2_OK);
    assert(ace_len > 0);
    assert(ace_len < sizeof(ace));
    ace[ace_len] = '\0';  /* idn2_punycode_encode does NOT NUL-terminate. */

    /* Verify against the Punycode for this exact codepoint sequence.
     * (Encoding cross-checked against libidn2; note U+03C2 is final
     * sigma "ς"; U+03C3 "σ" would yield "nxasmq6b".) */
    assert(strcmp(ace, "nxasmm1c") == 0);

    uint32_t roundtrip[16];
    size_t roundtrip_len = sizeof(roundtrip) / sizeof(roundtrip[0]);
    rc = idn2_punycode_decode(ace, strlen(ace), roundtrip, &roundtrip_len);
    assert(rc == IDN2_OK);
    assert(roundtrip_len == unicode_in_len);
    for (size_t i = 0; i < unicode_in_len; i++) {
        assert(roundtrip[i] == unicode_in[i]);
    }

    printf("libidn2 punycode OK: encoded=%s roundtrip_len=%zu\n",
           ace, roundtrip_len);
    return 0;
}
