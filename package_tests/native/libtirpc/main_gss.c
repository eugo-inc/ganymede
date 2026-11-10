/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtirpc: gss; calls rpc_gss_get_mechanisms(), rpc_gss_get_mech_info(), rpc_gss_qop_to_num(), rpc_gss_get_versions().
 */

#include <tirpc/rpc/rpc.h>
#include <tirpc/rpc/rpcsec_gss.h>
#include <tirpc/rpc/auth_gss.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* 1. The mechanism table: exactly one mechanism, Kerberos v5. */
    char **mechanisms = rpc_gss_get_mechanisms();
    assert(mechanisms != NULL);
    assert(mechanisms[0] != NULL && strcmp(mechanisms[0], "kerberos_v5") == 0);
    assert(mechanisms[1] == NULL);

    /* 2. Its quality-of-protection list and versions. */
    rpc_gss_service_t service = rpcsec_gss_svc_none;
    char **qops = rpc_gss_get_mech_info("kerberos_v5", &service);
    assert(qops != NULL && qops[0] != NULL && strcmp(qops[0], "GSS_C_QOP_DEFAULT") == 0);
    assert(service == rpcsec_gss_svc_privacy);
    u_int qop = 1;
    assert(rpc_gss_qop_to_num("GSS_C_QOP_DEFAULT", "kerberos_v5", &qop));
    assert(qop == 0);
    u_int high = 0, low = 0;
    assert(rpc_gss_get_versions(&high, &low));
    assert(low >= 1 && high >= low);

    setenv("KRB5CCNAME", "FILE:/nonexistent/demo_libtirpc_test_ccache", 1);
    CLIENT *client = clnt_raw_create(100003, 4);
    assert(client != NULL);
    struct rpc_gss_sec security;
    memset(&security, 0, sizeof(security));
    static gss_OID_desc kerberos_v5_oid = {9, "\052\206\110\206\367\022\001\002\002"};
    security.mech = &kerberos_v5_oid;
    security.qop = GSS_C_QOP_DEFAULT;
    security.svc = RPCSEC_GSS_SVC_NONE;
    AUTH *auth = authgss_create_default(client, "nfs@localhost", &security);
    assert(auth == NULL);

    printf("libtirpc RPCSEC_GSS test OK (mechanism %s, versions %u-%u)\n", mechanisms[0], low, high);
    return 0;
}
