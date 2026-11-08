/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for ucc: core API; calls ucc_status_string(), ucc_lib_config_read(), ucc_init(), ucc_lib_config_release().
 */

#include <ucc/api/ucc.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_UCC_PROBE_COUNT 64

/* The one-process out-of-band allgather: "gather" from a team of one is a copy. */
static ucc_status_t demo_oob_allgather(void *src_buf, void *recv_buf, size_t size, void *allgather_info,
                                       void **request)
{
    (void)allgather_info;
    memcpy(recv_buf, src_buf, size);
    *request = NULL;
    return UCC_OK;
}

static ucc_status_t demo_oob_req_test(void *request)
{
    (void)request;
    return UCC_OK;
}

static ucc_status_t demo_oob_req_free(void *request)
{
    (void)request;
    return UCC_OK;
}

static void demo_fail(const char *step, ucc_status_t status)
{
    fprintf(stderr, "UCC-PROBE: FAIL [%s] %s\n", step, ucc_status_string(status));
    exit(1);
}

int main(void)
{
    ucc_lib_config_h     lib_config;
    ucc_context_config_h context_config;
    ucc_lib_h            lib;
    ucc_context_h        context;
    ucc_team_h           team;
    ucc_coll_req_h       request;
    ucc_status_t         status;
    ucc_oob_coll_t       oob;
    ucc_lib_params_t     lib_params;
    ucc_context_params_t context_params;
    ucc_team_params_t    team_params;
    ucc_coll_args_t      args;
    int32_t              source[DEMO_UCC_PROBE_COUNT];
    int32_t              result[DEMO_UCC_PROBE_COUNT];
    unsigned             major, minor, release;
    int                  i;

    memset(&oob, 0, sizeof(oob));
    oob.allgather = demo_oob_allgather;
    oob.req_test  = demo_oob_req_test;
    oob.req_free  = demo_oob_req_free;
    oob.coll_info = NULL;
    oob.n_oob_eps = 1;
    oob.oob_ep    = 0;

    memset(&lib_params, 0, sizeof(lib_params));
    lib_params.mask        = UCC_LIB_PARAM_FIELD_THREAD_MODE;
    lib_params.thread_mode = UCC_THREAD_SINGLE;
    status = ucc_lib_config_read(NULL, NULL, &lib_config);
    if (status != UCC_OK) demo_fail("lib_config_read", status);
    status = ucc_init(&lib_params, lib_config, &lib);
    ucc_lib_config_release(lib_config);
    if (status != UCC_OK) demo_fail("init", status);

    status = ucc_context_config_read(lib, NULL, &context_config);
    if (status != UCC_OK) demo_fail("context_config_read", status);
    memset(&context_params, 0, sizeof(context_params));
    context_params.mask = UCC_CONTEXT_PARAM_FIELD_OOB;
    context_params.oob  = oob;
    status = ucc_context_create(lib, &context_params, context_config, &context);
    ucc_context_config_release(context_config);
    if (status != UCC_OK) demo_fail("context_create", status);

    memset(&team_params, 0, sizeof(team_params));
    team_params.mask     = UCC_TEAM_PARAM_FIELD_EP | UCC_TEAM_PARAM_FIELD_EP_RANGE | UCC_TEAM_PARAM_FIELD_OOB;
    team_params.ep       = 0;
    team_params.ep_range = UCC_COLLECTIVE_EP_RANGE_CONTIG;
    team_params.oob      = oob;
    status = ucc_team_create_post(&context, 1, &team_params, &team);
    if (status != UCC_OK) demo_fail("team_create_post", status);
    while ((status = ucc_team_create_test(team)) == UCC_INPROGRESS) {
        ucc_context_progress(context);
    }
    if (status != UCC_OK) demo_fail("team_create_test", status);

    for (i = 0; i < DEMO_UCC_PROBE_COUNT; i++) {
        source[i] = 3 * i + 1;
        result[i] = -1;
    }
    memset(&args, 0, sizeof(args));
    args.coll_type            = UCC_COLL_TYPE_ALLREDUCE;
    args.src.info.buffer      = source;
    args.src.info.count       = DEMO_UCC_PROBE_COUNT;
    args.src.info.datatype    = UCC_DT_INT32;
    args.src.info.mem_type    = UCC_MEMORY_TYPE_HOST;
    args.dst.info.buffer      = result;
    args.dst.info.count       = DEMO_UCC_PROBE_COUNT;
    args.dst.info.datatype    = UCC_DT_INT32;
    args.dst.info.mem_type    = UCC_MEMORY_TYPE_HOST;
    args.op                   = UCC_OP_SUM;
    status = ucc_collective_init(&args, &request, team);
    if (status != UCC_OK) demo_fail("collective_init", status);
    status = ucc_collective_post(request);
    if (status != UCC_OK) demo_fail("collective_post", status);
    while ((status = ucc_collective_test(request)) == UCC_INPROGRESS) {
        ucc_context_progress(context);
    }
    if (status != UCC_OK) demo_fail("collective_test", status);
    ucc_collective_finalize(request);

    /* A sum over a team of one is the input itself. */
    for (i = 0; i < DEMO_UCC_PROBE_COUNT; i++) {
        if (result[i] != source[i]) {
            fprintf(stderr, "UCC-PROBE: FAIL [allreduce] element %d is %d, expected %d\n", i, result[i], source[i]);
            return 1;
        }
    }

    while ((status = ucc_team_destroy(team)) == UCC_INPROGRESS) {
        ucc_context_progress(context);
    }
    if (status != UCC_OK) demo_fail("team_destroy", status);
    status = ucc_context_destroy(context);
    if (status != UCC_OK) demo_fail("context_destroy", status);
    status = ucc_finalize(lib);
    if (status != UCC_OK) demo_fail("finalize", status);

    ucc_get_version(&major, &minor, &release);
    printf("UCC-PROBE: PASS - UCC %u.%u.%u (%s) ran a one-process allreduce of %d int32 values in host memory\n",
           major, minor, release, ucc_get_version_string(), DEMO_UCC_PROBE_COUNT);
    return 0;
}
