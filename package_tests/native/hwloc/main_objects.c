/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for hwloc: objects; calls hwloc_topology_init(), hwloc_topology_load(), hwloc_obj_type_string(), hwloc_type_sscanf().
 */

#include <hwloc.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    hwloc_topology_t topo;
    assert(hwloc_topology_init(&topo) == 0);
    assert(hwloc_topology_load(topo) == 0);

    const char *pu_name = hwloc_obj_type_string(HWLOC_OBJ_PU);
    assert(pu_name != NULL);
    assert(strcmp(pu_name, "PU") == 0);

    hwloc_obj_type_t parsed_type = HWLOC_OBJ_MACHINE;
    int parse_rc = hwloc_type_sscanf(pu_name, &parsed_type, NULL, 0);
    assert(parse_rc == 0);
    assert(parsed_type == HWLOC_OBJ_PU);

    /* Root object: has a non-NULL cpuset, the cpuset is non-empty. */
    hwloc_obj_t root = hwloc_get_root_obj(topo);
    assert(root != NULL);
    assert(root->cpuset != NULL);
    assert(!hwloc_bitmap_iszero(root->cpuset));
    int root_weight = hwloc_bitmap_weight(root->cpuset);
    assert(root_weight >= 1);

    /* Iterate every PU; their cpusets must each be singletons and collectively
     * cover the root's cpuset (every bit in root.cpuset corresponds to exactly
     * one PU). */
    int pu_depth = hwloc_get_type_depth(topo, HWLOC_OBJ_PU);
    assert(pu_depth != HWLOC_TYPE_DEPTH_UNKNOWN);
    unsigned int nbpus = hwloc_get_nbobjs_by_depth(topo, pu_depth);
    assert(nbpus >= 1);
    assert((int)nbpus == root_weight);

    hwloc_bitmap_t accumulated = hwloc_bitmap_alloc();
    assert(accumulated != NULL);

    hwloc_obj_t pu = NULL;
    unsigned int seen = 0;
    while ((pu = hwloc_get_next_obj_by_depth(topo, pu_depth, pu)) != NULL) {
        assert(pu->type == HWLOC_OBJ_PU);
        assert(pu->cpuset != NULL);
        assert(hwloc_bitmap_weight(pu->cpuset) == 1);
        /* Each PU's os_index should match the single bit set in its cpuset. */
        assert((int)pu->os_index == hwloc_bitmap_first(pu->cpuset));
        hwloc_bitmap_or(accumulated, accumulated, pu->cpuset);
        seen++;
    }
    assert(seen == nbpus);
    /* Union of PU cpusets equals the root cpuset. */
    assert(hwloc_bitmap_isequal(accumulated, root->cpuset));

    /* The first PU must be reachable as a descendant of the root via the
     * generic ancestor walk. */
    hwloc_obj_t first_pu = hwloc_get_obj_by_depth(topo, pu_depth, 0);
    assert(first_pu != NULL);
    hwloc_obj_t ancestor = first_pu->parent;
    int parent_hops = 0;
    while (ancestor != NULL && ancestor != root) {
        ancestor = ancestor->parent;
        parent_hops++;
        assert(parent_hops < 64); /* defensive: no infinite chain */
    }
    assert(ancestor == root);

    hwloc_bitmap_free(accumulated);
    hwloc_topology_destroy(topo);

    printf("hwloc objects test OK: nbpus=%u, root_weight=%d\n", nbpus, root_weight);
    return 0;
}
