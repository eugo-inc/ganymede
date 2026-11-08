/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cpuinfo: topology; calls cpuinfo_initialize(), cpuinfo_get_processors_count(), cpuinfo_get_cores_count(), cpuinfo_get_packages_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(cpuinfo_initialize());

    const uint32_t n_proc = cpuinfo_get_processors_count();
    const uint32_t n_core = cpuinfo_get_cores_count();
    const uint32_t n_pkg = cpuinfo_get_packages_count();
    assert(n_proc >= 1 && n_core >= 1 && n_pkg >= 1);

    /* Bulk-pointer accessors must match the per-index accessor for index 0. */
    const struct cpuinfo_processor *proc_arr = cpuinfo_get_processors();
    const struct cpuinfo_core *core_arr = cpuinfo_get_cores();
    const struct cpuinfo_package *pkg_arr = cpuinfo_get_packages();
    assert(proc_arr != NULL);
    assert(core_arr != NULL);
    assert(pkg_arr != NULL);
    assert(&proc_arr[0] == cpuinfo_get_processor(0));
    assert(&core_arr[0] == cpuinfo_get_core(0));
    assert(&pkg_arr[0] == cpuinfo_get_package(0));

    /* Walk every logical processor and check topology back-pointers. */
    for (uint32_t i = 0; i < n_proc; i++) {
        const struct cpuinfo_processor *p = cpuinfo_get_processor(i);
        assert(p != NULL);
        assert(p->core != NULL);
        assert(p->package != NULL);

        /* This processor's index must fall inside its core's window. */
        const struct cpuinfo_core *c = p->core;
        assert(c->processor_count >= 1);
        assert(i >= c->processor_start);
        assert(i < c->processor_start + c->processor_count);

        /* And inside its package's window. */
        const struct cpuinfo_package *pkg = p->package;
        assert(pkg->processor_count >= 1);
        assert(i >= pkg->processor_start);
        assert(i < pkg->processor_start + pkg->processor_count);

        /* Core's package must agree with the processor's package. */
        assert(c->package == pkg);
    }

    /* Sum of per-package processor_counts must equal total processors. */
    uint32_t pkg_proc_sum = 0;
    for (uint32_t i = 0; i < n_pkg; i++) {
        const struct cpuinfo_package *pkg = cpuinfo_get_package(i);
        assert(pkg != NULL);
        pkg_proc_sum += pkg->processor_count;
    }
    assert(pkg_proc_sum == n_proc);

    cpuinfo_deinitialize();

    printf("cpuinfo topology test OK: %u procs across %u cores / %u pkgs\n",
           n_proc, n_core, n_pkg);
    return 0;
}
