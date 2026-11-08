/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for grpc: gpr; calls grpc_slice_from_copied_buffer(), grpc_slice_unref(), gpr_time_from_seconds(), gpr_time_cmp().
 */

#include <grpc/slice.h>
#include <grpc/support/alloc.h>
#include <grpc/support/string_util.h>
#include <grpc/support/time.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* (1) gpr_slice — the byte-buffer primitive. */
    const char *payload = "demo-grpc-probe";
    const size_t payload_len = strlen(payload);
    grpc_slice slice = grpc_slice_from_copied_buffer(payload, payload_len);
    assert(GRPC_SLICE_LENGTH(slice) == payload_len);
    assert(memcmp(GRPC_SLICE_START_PTR(slice), payload, payload_len) == 0);
    grpc_slice_unref(slice);

    /* (2) gpr_time arithmetic. */
    gpr_timespec t1 = gpr_time_from_seconds(5, GPR_TIMESPAN);
    gpr_timespec t2 = gpr_time_from_seconds(3, GPR_TIMESPAN);
    assert(gpr_time_cmp(t1, t2) > 0);
    gpr_timespec sum = gpr_time_add(t1, t2);
    /* 5s + 3s = 8s. */
    assert(sum.tv_sec == 8);

    gpr_timespec inf = gpr_inf_future(GPR_CLOCK_REALTIME);
    gpr_timespec now = gpr_now(GPR_CLOCK_REALTIME);
    assert(gpr_time_cmp(inf, now) > 0);

    /* (3) gpr_strdup / gpr_free — the portable heap primitives. */
    char *dup = gpr_strdup(payload);
    assert(dup != NULL);
    assert(dup != payload);
    assert(strcmp(dup, payload) == 0);
    gpr_free(dup);

    printf("grpc gpr test OK: payload_len=%zu sum_sec=%lld\n",
           payload_len, (long long)sum.tv_sec);
    return 0;
}
