/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for openssl: hmac; calls EVP_MAC_fetch(), EVP_MAC_CTX_new(), OSSL_PARAM_construct_utf8_string(), OSSL_PARAM_construct_end().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openssl/evp.h>
#include <openssl/params.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* RFC 4231 Test Case 1: key = 20 * 0x0b, data = "Hi There" */
    static const unsigned char key[20] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
    };
    static const unsigned char data[8] = { 'H', 'i', ' ', 'T', 'h', 'e', 'r', 'e' };
    static const unsigned char expected[32] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
        0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
        0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7,
    };

    EVP_MAC *mac = EVP_MAC_fetch(NULL, "HMAC", NULL);
    assert(mac != NULL);

    EVP_MAC_CTX *ctx = EVP_MAC_CTX_new(mac);
    assert(ctx != NULL);

    int rc = 0;

    /* Tell HMAC which digest to use. The OSSL_PARAM array is the modern ABI. */
    char digest_name[] = "SHA256";
    OSSL_PARAM params[2];
    params[0] = OSSL_PARAM_construct_utf8_string("digest", digest_name, 0);
    params[1] = OSSL_PARAM_construct_end();

    rc = EVP_MAC_init(ctx, key, sizeof(key), params);
    assert(rc == 1);
    rc = EVP_MAC_update(ctx, data, sizeof(data));
    assert(rc == 1);

    unsigned char out[32];
    size_t outlen = 0;
    rc = EVP_MAC_final(ctx, out, &outlen, sizeof(out));
    assert(rc == 1);
    assert(outlen == sizeof(expected));
    assert(memcmp(out, expected, sizeof(expected)) == 0);

    EVP_MAC_CTX_free(ctx);
    EVP_MAC_free(mac);

    printf("openssl HMAC-SHA256 (RFC 4231 TC1) OK\n");
    return 0;
}
