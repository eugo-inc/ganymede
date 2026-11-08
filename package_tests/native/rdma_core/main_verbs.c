/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rdma_core: verbs; calls ibv_wc_status_str().
 */

#include <infiniband/efadv.h>
#include <infiniband/mlx5dv.h>
#include <infiniband/verbs.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    /* A libibverbs entry point that needs no kernel: proves the library loaded and resolves its own symbols. */
    const char *success_text = ibv_wc_status_str(IBV_WC_SUCCESS);
    if (success_text == NULL || strcmp(success_text, "success") != 0) {
        fprintf(stderr, "ibv_wc_status_str(IBV_WC_SUCCESS) returned '%s', expected 'success'\n",
                success_text != NULL ? success_text : "(null)");
        return 1;
    }

    /* Taking the addresses forces the linker to resolve both provider exports. */
    void *volatile mlx5_query = (void *)&mlx5dv_query_device;
    void *volatile efa_query = (void *)&efadv_query_device;
    if (mlx5_query == NULL || efa_query == NULL) {
        fprintf(stderr, "a provider entry point resolved to NULL\n");
        return 1;
    }

    printf("rdma-core: libibverbs answered ibv_wc_status_str; mlx5dv_query_device and efadv_query_device linked\n");
    return 0;
}
