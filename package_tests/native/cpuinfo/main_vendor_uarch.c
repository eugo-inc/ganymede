/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cpuinfo: vendor uarch; calls cpuinfo_initialize(), cpuinfo_get_cores_count(), cpuinfo_get_core(), cpuinfo_get_processors_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(cpuinfo_initialize());

    const uint32_t n_core = cpuinfo_get_cores_count();
    assert(n_core >= 1);
    const struct cpuinfo_core *c0 = cpuinfo_get_core(0);
    assert(c0 != NULL);

    /* Per-core fields exist + window fits inside the machine. */
    assert(c0->processor_count >= 1);
    assert(c0->processor_start + c0->processor_count
           <= cpuinfo_get_processors_count());

    /* Package back-pointer is populated (unique-to-core-struct linkage). */
    assert(c0->package != NULL);

    /* Vendor and microarchitecture are enum codes; any vendor is accepted. */
    const enum cpuinfo_vendor v0 = c0->vendor;
    const enum cpuinfo_uarch u0 = c0->uarch;
    assert((int)v0 >= 0);
    assert((int)u0 >= 0);

#if CPUINFO_ARCH_X86_64
    /* CPUID leaf 1 EAX is the per-core raw register field on x86_64. */
    (void)c0->cpuid;
#elif CPUINFO_ARCH_ARM64
    /* MIDR_EL1 is the per-core raw register field on arm64. */
    (void)c0->midr;
#endif

    cpuinfo_deinitialize();

    printf("cpuinfo vendor/uarch test OK: vendor=%d uarch=%d n_core=%u\n",
           (int)v0, (int)u0, n_core);
    return 0;
}
