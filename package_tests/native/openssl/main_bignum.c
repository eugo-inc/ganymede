/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openssl: bignum; calls BN_CTX_new(), BN_dec2bn(), BN_new(), BN_mod_exp().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <openssl/bn.h>
#include <openssl/crypto.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    int rc = 0;
    BN_CTX *ctx = BN_CTX_new();
    assert(ctx != NULL);

    BIGNUM *base = NULL, *exp = NULL, *mod = NULL, *result = NULL;
    rc = BN_dec2bn(&base, "2");
    assert(rc != 0);
    rc = BN_dec2bn(&exp,  "16");
    assert(rc != 0);
    rc = BN_dec2bn(&mod,  "17");
    assert(rc != 0);
    result = BN_new();
    assert(result != NULL);

    /* Fermat's little theorem: a^(p-1) mod p = 1 when gcd(a,p)=1, p prime. */
    rc = BN_mod_exp(result, base, exp, mod, ctx);
    assert(rc == 1);

    char *dec = BN_bn2dec(result);
    assert(dec != NULL);
    assert(strcmp(dec, "1") == 0);
    OPENSSL_free(dec);

    BIGNUM *big = NULL;
    const char *big_in = "123456789012345678901234567890";
    rc = BN_dec2bn(&big, big_in);
    assert(rc != 0);
    char *big_dec = BN_bn2dec(big);
    assert(big_dec != NULL);
    assert(strcmp(big_dec, big_in) == 0);
    OPENSSL_free(big_dec);

    BIGNUM *hex_bn = NULL;
    rc = BN_hex2bn(&hex_bn, "DEADBEEF");
    assert(rc != 0);
    char *hex_str = BN_bn2hex(hex_bn);
    assert(hex_str != NULL);
    /* BN_bn2hex returns uppercase; compare case-insensitively via strcasecmp-free path. */
    assert(strlen(hex_str) == 8);
    assert(strcmp(hex_str, "DEADBEEF") == 0);
    OPENSSL_free(hex_str);

    BN_free(base);
    BN_free(exp);
    BN_free(mod);
    BN_free(result);
    BN_free(big);
    BN_free(hex_bn);
    BN_CTX_free(ctx);

    printf("openssl BIGNUM arithmetic + I/O OK\n");
    return 0;
}
