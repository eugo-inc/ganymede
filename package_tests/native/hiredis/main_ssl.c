/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hiredis: ssl; calls redisInitOpenSSL(), redisCreateSSLContext(), redisSSLContextGetError(), redisFreeSSLContext().
 *
 * No network and no certificates are needed:
 *   1. redisInitOpenSSL() must return REDIS_OK.
 *   2. redisCreateSSLContext(NULL, ...) must return a context. With no CA file and no CA
 *      directory, hiredis asks OpenSSL for its default verify paths, which succeeds even
 *      when no CA bundle is installed, so NULL here means libhiredis_ssl or OpenSSL is broken.
 *   3. A CA file that does not exist must give NULL, REDIS_SSL_CTX_CA_CERT_LOAD_FAILED and a
 *      non-empty message from redisSSLContextGetError().
 */

#include <hiredis/hiredis.h>
#include <hiredis/hiredis_ssl.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Step 1: initialize OpenSSL through the hiredis_ssl wrapper. */
    if (redisInitOpenSSL() != REDIS_OK) {
        fprintf(stderr, "FAIL: redisInitOpenSSL() did not return REDIS_OK\n");
        return 1;
    }

    /* Step 2: a default SSL context must be created and freed. */
    redisSSLContextError err = REDIS_SSL_CTX_NONE;
    redisSSLContext *ctx = redisCreateSSLContext(NULL, NULL, NULL, NULL, NULL, &err);
    if (ctx == NULL) {
        const char *msg = redisSSLContextGetError(err);
        fprintf(stderr, "FAIL: redisCreateSSLContext(NULL, ...) returned NULL (err=%d: %s)\n",
                (int)err, msg ? msg : "(null)");
        return 1;
    }
    if (err != REDIS_SSL_CTX_NONE) {
        fprintf(stderr, "FAIL: redisCreateSSLContext returned a context but err=%d\n", (int)err);
        redisFreeSSLContext(ctx);
        return 1;
    }
    redisFreeSSLContext(ctx);

    /* Step 3: the error path, with a CA file that cannot exist. */
    err = REDIS_SSL_CTX_NONE;
    ctx = redisCreateSSLContext("/nonexistent/demo-ca.pem", NULL, NULL, NULL, NULL, &err);
    if (ctx != NULL) {
        fprintf(stderr, "FAIL: redisCreateSSLContext accepted a CA file that does not exist\n");
        redisFreeSSLContext(ctx);
        return 1;
    }
    if (err != REDIS_SSL_CTX_CA_CERT_LOAD_FAILED) {
        fprintf(stderr, "FAIL: missing CA file gave err=%d, want REDIS_SSL_CTX_CA_CERT_LOAD_FAILED (%d)\n",
                (int)err, (int)REDIS_SSL_CTX_CA_CERT_LOAD_FAILED);
        return 1;
    }
    const char *msg = redisSSLContextGetError(err);
    if (msg == NULL || strlen(msg) == 0) {
        fprintf(stderr, "FAIL: redisSSLContextGetError gave no message for err=%d\n", (int)err);
        return 1;
    }

    printf("hiredis ssl test OK (default ctx created and freed; missing CA -> \"%s\")\n", msg);
    return 0;
}
