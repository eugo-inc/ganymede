/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpfm4: core API; calls pfm_strerror(), pfm_initialize(), pfm_get_version(), pfm_get_pmu_info().
 */

#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>
#include <stdio.h>
#include <string.h>

static int fail(const char *what, int rc) {
    fprintf(stderr, "libpfm4 test FAILED: %s (rc=%d: %s)\n", what, rc, pfm_strerror(rc));
    return 1;
}

int main(void) {
    int rc = pfm_initialize();
    if (rc != PFM_SUCCESS) return fail("pfm_initialize", rc);

    int version = pfm_get_version();
    if (PFM_MAJ_VERSION(version) != PFM_MAJ_VERSION(LIBPFM_VERSION)) {
        fprintf(stderr, "libpfm4 test FAILED: the loaded library is %d.%d but the headers are %d.%d\n",
                PFM_MAJ_VERSION(version), PFM_MIN_VERSION(version),
                PFM_MAJ_VERSION(LIBPFM_VERSION), PFM_MIN_VERSION(LIBPFM_VERSION));
        return 1;
    }

    pfm_pmu_info_t pinfo;
    memset(&pinfo, 0, sizeof(pinfo));
    pinfo.size = sizeof(pinfo);
    rc = pfm_get_pmu_info(PFM_PMU_PERF_EVENT, &pinfo);
    if (rc != PFM_SUCCESS) return fail("pfm_get_pmu_info(PFM_PMU_PERF_EVENT)", rc);
    if (!pinfo.is_present) {
        fprintf(stderr, "libpfm4 test FAILED: the generic perf PMU is not present\n");
        return 1;
    }

    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    pfm_perf_encode_arg_t arg;
    memset(&arg, 0, sizeof(arg));
    arg.attr = &attr;
    arg.size = sizeof(arg);
    rc = pfm_get_os_event_encoding("perf::PERF_COUNT_HW_CPU_CYCLES", PFM_PLM3, PFM_OS_PERF_EVENT, &arg);
    if (rc != PFM_SUCCESS) return fail("pfm_get_os_event_encoding(perf::PERF_COUNT_HW_CPU_CYCLES)", rc);
    if (attr.type != PERF_TYPE_HARDWARE || attr.config != PERF_COUNT_HW_CPU_CYCLES) {
        fprintf(stderr, "libpfm4 test FAILED: cycles encoded to type=%u config=%llu, expected %u/%u\n",
                attr.type, (unsigned long long)attr.config, PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        return 1;
    }

    int present = 0;
    pfm_pmu_t pmu;
    pfm_for_all_pmus(pmu) {
        pfm_pmu_info_t info;
        memset(&info, 0, sizeof(info));
        info.size = sizeof(info);
        if (pfm_get_pmu_info(pmu, &info) == PFM_SUCCESS && info.is_present) present++;
    }

    pfm_terminate();

    printf("libpfm4 test OK: library %d.%d, %d PMU(s) present, cycles -> type %u config %llu\n",
           PFM_MAJ_VERSION(version), PFM_MIN_VERSION(version), present, attr.type,
           (unsigned long long)attr.config);
    return 0;
}
