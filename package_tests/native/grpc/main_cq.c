/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for grpc: cq; calls grpc_init(), grpc_completion_queue_create_for_next(), gpr_time_0(), grpc_completion_queue_next().
 */

#include <grpc/grpc.h>
#include <grpc/support/log.h>
#include <grpc/support/time.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    grpc_init();

    grpc_completion_queue *cq = grpc_completion_queue_create_for_next(NULL);
    assert(cq != NULL);

    /* (1) Empty CQ + immediate deadline => QUEUE_TIMEOUT, not COMPLETE. */
    gpr_timespec deadline = gpr_time_0(GPR_CLOCK_REALTIME);
    grpc_event ev = grpc_completion_queue_next(cq, deadline, NULL);
    assert(ev.type == GRPC_QUEUE_TIMEOUT);

    /* (2) Shutdown handshake. After shutdown, draining yields QUEUE_SHUTDOWN. */
    grpc_completion_queue_shutdown(cq);
    gpr_timespec inf = gpr_inf_future(GPR_CLOCK_REALTIME);
    grpc_event drained = grpc_completion_queue_next(cq, inf, NULL);
    assert(drained.type == GRPC_QUEUE_SHUTDOWN);

    /* (3) Clean destroy. */
    grpc_completion_queue_destroy(cq);

    grpc_shutdown();

    printf("grpc cq test OK: timeout->shutdown handshake clean\n");
    return 0;
}
