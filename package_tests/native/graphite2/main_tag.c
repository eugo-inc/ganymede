/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for graphite2: tag; calls gr_str_to_tag(), gr_tag_to_str().
 */

#include <graphite2/Font.h>
#include <graphite2/Types.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    gr_uint32 tag_latn = gr_str_to_tag("latn");
    char round[8];
    memset(round, '\xAB', sizeof(round)); /* poison */
    gr_tag_to_str(tag_latn, round);
    assert(round[0] == 'l');
    assert(round[1] == 'a');
    assert(round[2] == 't');
    assert(round[3] == 'n');
    assert(round[4] == 0); /* gr_tag_to_str appends a trailing NUL */

    /* (2) Big-endian packing: first char lives in the most-significant byte.
     * "latn" = 'l'(0x6C), 'a'(0x61), 't'(0x74), 'n'(0x6E) -> 0x6C61746E. */
    assert(tag_latn == 0x6C61746Eu);

    char en_in[5] = {'e', 'n', 0, 0, 0};
    gr_uint32 tag_en = gr_str_to_tag(en_in);
    assert(tag_en == 0x656E0000u);

    char short_round[8];
    memset(short_round, '\xAB', sizeof(short_round));
    gr_tag_to_str(tag_en, short_round);
    assert(short_round[0] == 'e');
    assert(short_round[1] == 'n');
    assert(short_round[2] == 0);
    assert(short_round[3] == 0);
    assert(short_round[4] == 0); /* trailing NUL */

    gr_uint32 tag_smcp = gr_str_to_tag("smcp");
    assert(tag_smcp == 0x736D6370u);
    char r2[8];
    memset(r2, '\xAB', sizeof(r2));
    gr_tag_to_str(tag_smcp, r2);
    assert(memcmp(r2, "smcp", 4) == 0);

    printf("graphite2 tag test OK: latn=0x%08x en=0x%08x smcp=0x%08x\n",
           tag_latn, tag_en, tag_smcp);
    return 0;
}
