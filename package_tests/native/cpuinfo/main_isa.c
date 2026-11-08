/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for cpuinfo: isa; calls cpuinfo_initialize(), cpuinfo_has_x86_sse2(), cpuinfo_has_x86_sse3(), cpuinfo_has_x86_ssse3().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(cpuinfo_initialize());

#if CPUINFO_ARCH_X86_64
    /* SSE2 is mandated by the x86_64 ABI — present on every x86_64 CPU. */
    assert(cpuinfo_has_x86_sse2());
    /* The other feature flags vary by CPU model; report them without asserting. */
    const int sse3 = cpuinfo_has_x86_sse3() ? 1 : 0;
    const int ssse3 = cpuinfo_has_x86_ssse3() ? 1 : 0;
    const int cmov = cpuinfo_has_x86_cmov() ? 1 : 0;
    const int popcnt = cpuinfo_has_x86_popcnt() ? 1 : 0;
    printf("cpuinfo isa test OK (x86_64): sse2=1 sse3=%d ssse3=%d cmov=%d popcnt=%d\n",
           sse3, ssse3, cmov, popcnt);
#elif CPUINFO_ARCH_ARM64
    /* NEON (ASIMD) is mandatory on AArch64 — present on every arm64 CPU. */
    assert(cpuinfo_has_arm_neon());
    printf("cpuinfo isa test OK (arm64): neon present\n");
#else
    printf("cpuinfo isa test OK: no ISA assertion for this architecture\n");
#endif

    cpuinfo_deinitialize();
    return 0;
}
