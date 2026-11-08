/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libmd: md4; calls MD4Init(), MD4Update(), MD4Final().
 */

#include <md4.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char expected_md4[MD4_DIGEST_LENGTH] = {
        0x86, 0x64, 0x37, 0xcb, 0x7a, 0x79, 0x4b, 0xce,
        0x2b, 0x72, 0x7a, 0xcc, 0x03, 0x62, 0xee, 0x27,
    };
    unsigned char md4_digest[MD4_DIGEST_LENGTH];
    MD4_CTX md4_ctx;
    MD4Init(&md4_ctx);
    MD4Update(&md4_ctx, (const unsigned char *)"hello", 5);
    MD4Final(md4_digest, &md4_ctx);
    assert(memcmp(md4_digest, expected_md4, sizeof(md4_digest)) == 0);

    MD4_CTX md4_ctx2;
    unsigned char md4_digest2[MD4_DIGEST_LENGTH];
    MD4Init(&md4_ctx2);
    MD4Update(&md4_ctx2, (const unsigned char *)"hel", 3);
    MD4Update(&md4_ctx2, (const unsigned char *)"lo", 2);
    MD4Final(md4_digest2, &md4_ctx2);
    assert(memcmp(md4_digest2, expected_md4, sizeof(md4_digest2)) == 0);

    printf("libmd md4 test OK\n");
    return 0;
}
