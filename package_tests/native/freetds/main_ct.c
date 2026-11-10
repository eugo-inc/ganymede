/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for freetds: ct; calls cs_ctx_alloc(), ct_init(), ct_con_alloc(), ct_con_props().
 */

#include <ctpublic.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    CS_CONTEXT *ctx = NULL;
    if (cs_ctx_alloc(CS_VERSION_100, &ctx) != CS_SUCCEED || ctx == NULL) {
        fprintf(stderr, "FAIL: cs_ctx_alloc(CS_VERSION_100) failed\n");
        return 1;
    }
    if (ct_init(ctx, CS_VERSION_100) != CS_SUCCEED) {
        fprintf(stderr, "FAIL: ct_init(CS_VERSION_100) failed\n");
        return 1;
    }

    CS_CONNECTION *conn = NULL;
    if (ct_con_alloc(ctx, &conn) != CS_SUCCEED || conn == NULL) {
        fprintf(stderr, "FAIL: ct_con_alloc failed\n");
        return 1;
    }
    const char *user = "demo_probe";
    if (ct_con_props(conn, CS_SET, CS_USERNAME, (CS_VOID *)user, CS_NULLTERM, NULL) != CS_SUCCEED) {
        fprintf(stderr, "FAIL: ct_con_props(CS_SET, CS_USERNAME) failed\n");
        return 1;
    }
    char got[64];
    CS_INT got_len = 0;
    memset(got, 0, sizeof got);
    if (ct_con_props(conn, CS_GET, CS_USERNAME, got, (CS_INT)(sizeof got - 1), &got_len) != CS_SUCCEED) {
        fprintf(stderr, "FAIL: ct_con_props(CS_GET, CS_USERNAME) failed\n");
        return 1;
    }
    if (strcmp(got, user) != 0) {
        fprintf(stderr, "FAIL: CS_USERNAME read back \"%s\" (len %d), want \"%s\"\n", got, (int)got_len, user);
        return 1;
    }

    if (ct_con_drop(conn) != CS_SUCCEED || ct_exit(ctx, CS_UNUSED) != CS_SUCCEED ||
        cs_ctx_drop(ctx) != CS_SUCCEED) {
        fprintf(stderr, "FAIL: Client-Library teardown (ct_con_drop / ct_exit / cs_ctx_drop) failed\n");
        return 1;
    }
    printf("freetds libct test OK\n");
    return 0;
}
