/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cpuinfo: current; calls cpuinfo_initialize(), cpuinfo_get_current_processor(), cpuinfo_get_current_core(), cpuinfo_get_processors_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(cpuinfo_initialize());

    const struct cpuinfo_processor *cur_p = cpuinfo_get_current_processor();
    const struct cpuinfo_core *cur_c = cpuinfo_get_current_core();
    assert(cur_p != NULL);
    assert(cur_c != NULL);

    /* The current processor must belong to the current core. */
    assert(cur_p->core == cur_c);

    /* Cross-check: current_p must be one of the descriptors in the global
     * processor array — catches a library that returns a stale pointer. */
    const uint32_t n_proc = cpuinfo_get_processors_count();
    const struct cpuinfo_processor *proc_arr = cpuinfo_get_processors();
    assert(proc_arr != NULL);
    int found = 0;
    for (uint32_t i = 0; i < n_proc; i++) {
        if (&proc_arr[i] == cur_p) {
            found = 1;
            break;
        }
    }
    assert(found);

    const uint32_t n_uarch = cpuinfo_get_uarchs_count();
    assert(n_uarch >= 1);
    const uint32_t sentinel = 0xFFFFFFFFu;
    const uint32_t ui = cpuinfo_get_current_uarch_index_with_default(sentinel);
    assert(ui < n_uarch || ui == sentinel);

    /* The uarch at index 0 must be reachable, and the bulk accessor must
     * match the per-index accessor. */
    const struct cpuinfo_uarch_info *u0 = cpuinfo_get_uarch(0);
    const struct cpuinfo_uarch_info *uarchs = cpuinfo_get_uarchs();
    assert(u0 != NULL);
    assert(uarchs != NULL);
    assert(&uarchs[0] == u0);

    cpuinfo_deinitialize();

    printf("cpuinfo current test OK: cur_proc=%p cur_core=%p uarch_idx=%u\n",
           (const void *)cur_p, (const void *)cur_c, ui);
    return 0;
}
