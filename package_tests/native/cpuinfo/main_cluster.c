/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cpuinfo: cluster; calls cpuinfo_initialize(), cpuinfo_get_clusters_count(), cpuinfo_get_processors_count(), cpuinfo_get_cores_count().
 */

#include <cpuinfo.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    assert(cpuinfo_initialize());

    const uint32_t n_cluster = cpuinfo_get_clusters_count();
    const uint32_t n_proc = cpuinfo_get_processors_count();
    const uint32_t n_core = cpuinfo_get_cores_count();
    const uint32_t n_pkg = cpuinfo_get_packages_count();
    assert(n_cluster >= 1);
    /* Every cluster holds at least one core, so there are never more clusters than cores. */
    assert(n_cluster <= n_core);
    /* And at least one cluster per package. */
    assert(n_cluster >= n_pkg);

    /* Bulk accessor must agree with the per-index accessor for index 0. */
    const struct cpuinfo_cluster *cluster_arr = cpuinfo_get_clusters();
    assert(cluster_arr != NULL);
    assert(&cluster_arr[0] == cpuinfo_get_cluster(0));

    /* Walk every cluster: window sanity + back-pointer to its package. */
    uint32_t total_cluster_procs = 0;
    uint32_t total_cluster_cores = 0;
    for (uint32_t i = 0; i < n_cluster; i++) {
        const struct cpuinfo_cluster *c = cpuinfo_get_cluster(i);
        assert(c != NULL);
        assert(c->processor_count >= 1);
        assert(c->core_count >= 1);
        assert(c->package != NULL);

        /* Cluster windows must fall inside the global processor/core arrays. */
        assert(c->processor_start + c->processor_count <= n_proc);
        assert(c->core_start + c->core_count <= n_core);

        /* The first core in this cluster must point back to it. */
        const struct cpuinfo_core *first_core = cpuinfo_get_core(c->core_start);
        assert(first_core != NULL);
        assert(first_core->cluster == c);
        /* And the package of that core must agree with the cluster's. */
        assert(first_core->package == c->package);

        total_cluster_procs += c->processor_count;
        total_cluster_cores += c->core_count;
    }

    /* Sum of cluster windows must cover the whole machine. */
    assert(total_cluster_procs == n_proc);
    assert(total_cluster_cores == n_core);

    cpuinfo_deinitialize();

    printf("cpuinfo cluster test OK: %u clusters (%u cores / %u procs)\n",
           n_cluster, total_cluster_cores, total_cluster_procs);
    return 0;
}
