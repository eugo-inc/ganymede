/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hwloc: core API; calls hwloc_get_api_version(), hwloc_topology_init(), hwloc_topology_load(), hwloc_topology_get_depth().
 */

#include <hwloc.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    /* Runtime API version must be non-zero and agree with the compile-time macro family. */
    unsigned int api_version = hwloc_get_api_version();
    assert(api_version != 0);
    assert((api_version >> 16) == (HWLOC_API_VERSION >> 16));

    hwloc_topology_t topology;
    assert(hwloc_topology_init(&topology) == 0);
    assert(hwloc_topology_load(topology) == 0);

    /* Every topology has at least the machine root object. */
    int topo_depth = hwloc_topology_get_depth(topology);
    assert(topo_depth >= 1);

    hwloc_obj_t root = hwloc_get_root_obj(topology);
    assert(root != NULL);
    assert(root->type == HWLOC_OBJ_MACHINE);

    /* At least one PU (processing unit) must be discoverable on any sane Linux host. */
    int pu_depth = hwloc_get_type_depth(topology, HWLOC_OBJ_PU);
    assert(pu_depth != HWLOC_TYPE_DEPTH_UNKNOWN);
    unsigned int nbpus = hwloc_get_nbobjs_by_depth(topology, pu_depth);
    assert(nbpus >= 1);

    hwloc_topology_destroy(topology);

    printf("hwloc test OK: api_version=0x%x\n", api_version);
    return 0;
}
