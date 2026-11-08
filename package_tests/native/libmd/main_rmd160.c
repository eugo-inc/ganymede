/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmd: rmd160; calls RMD160Init(), RMD160Update(), RMD160Final(), RMD160Data().
 */

#include <rmd160.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* (1) Streaming API: Init / Update / Final. */
    static const unsigned char expected_rmd160[RMD160_DIGEST_LENGTH] = {
        0x10, 0x8f, 0x07, 0xb8, 0x38, 0x24, 0x12, 0x61,
        0x2c, 0x04, 0x8d, 0x07, 0xd1, 0x3f, 0x81, 0x41,
        0x18, 0x44, 0x5a, 0xcd,
    };
    unsigned char rmd160_digest[RMD160_DIGEST_LENGTH];
    RMD160_CTX rmd_ctx;
    RMD160Init(&rmd_ctx);
    RMD160Update(&rmd_ctx, (const unsigned char *)"hello", 5);
    RMD160Final(rmd160_digest, &rmd_ctx);
    assert(memcmp(rmd160_digest, expected_rmd160, sizeof(rmd160_digest)) == 0);

    /* (2) One-shot API: RMD160Data() writes a hex string to a caller-provided
     * RMD160_DIGEST_STRING_LENGTH buffer and returns it. This is a STRUCTURALLY
     * different surface than Init/Update/Final — main.c does not exercise it. */
    char hex[RMD160_DIGEST_STRING_LENGTH];
    char *ret = RMD160Data((const unsigned char *)"hello", 5, hex);
    assert(ret == hex);
    assert(strcmp(hex, "108f07b8382412612c048d07d13f814118445acd") == 0);

    printf("libmd rmd160 test OK\n");
    return 0;
}
