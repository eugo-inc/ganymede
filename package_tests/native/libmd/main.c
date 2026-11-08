/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libmd: core API; calls MD5Init(), MD5Update(), MD5Final(), SHA1Init().
 */

#include <md5.h>
#include <sha1.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const unsigned char expected_md5[MD5_DIGEST_LENGTH] = {
        0x5d, 0x41, 0x40, 0x2a, 0xbc, 0x4b, 0x2a, 0x76,
        0xb9, 0x71, 0x9d, 0x91, 0x10, 0x17, 0xc5, 0x92,
    };
    unsigned char md5_digest[MD5_DIGEST_LENGTH];
    MD5_CTX md5_ctx;
    MD5Init(&md5_ctx);
    MD5Update(&md5_ctx, (const unsigned char *)"hello", 5);
    MD5Final(md5_digest, &md5_ctx);
    assert(memcmp(md5_digest, expected_md5, sizeof(md5_digest)) == 0);

    static const unsigned char expected_sha1[SHA1_DIGEST_LENGTH] = {
        0xaa, 0xf4, 0xc6, 0x1d, 0xdc, 0xc5, 0xe8, 0xa2,
        0xda, 0xbe, 0xde, 0x0f, 0x3b, 0x48, 0x2c, 0xd9,
        0xae, 0xa9, 0x43, 0x4d,
    };
    unsigned char sha1_digest[SHA1_DIGEST_LENGTH];
    SHA1_CTX sha1_ctx;
    SHA1Init(&sha1_ctx);
    SHA1Update(&sha1_ctx, (const unsigned char *)"hello", 5);
    SHA1Final(sha1_digest, &sha1_ctx);
    assert(memcmp(sha1_digest, expected_sha1, sizeof(sha1_digest)) == 0);

    printf("libmd test OK\n");
    return 0;
}
