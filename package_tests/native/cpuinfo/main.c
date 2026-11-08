/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cpuinfo: core API; calls cpuinfo_initialize(), cpuinfo_get_processors_count(), cpuinfo_get_cores_count(), cpuinfo_get_packages_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Initialization scans /proc/cpuinfo + CPUID; must succeed. */
    assert(cpuinfo_initialize());

    const uint32_t processors = cpuinfo_get_processors_count();
    const uint32_t cores = cpuinfo_get_cores_count();
    const uint32_t packages = cpuinfo_get_packages_count();
    assert(processors >= 1);
    assert(cores >= 1);
    assert(packages >= 1);
    assert(processors >= cores);
    assert(cores >= packages);

    /* The descriptor for processor 0 must be reachable — catches a bad
     * table-population path even if the counts above look right. */
    const struct cpuinfo_processor *p0 = cpuinfo_get_processor(0);
    assert(p0 != NULL);

    cpuinfo_deinitialize();

    printf("cpuinfo test OK: processors=%u cores=%u packages=%u\n",
           processors, cores, packages);
    return 0;
}
