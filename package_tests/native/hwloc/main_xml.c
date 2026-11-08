/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hwloc: xml; calls hwloc_topology_init(), hwloc_topology_load(), hwloc_topology_get_depth(), hwloc_get_type_depth().
 */

#include <hwloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    hwloc_topology_t orig;
    assert(hwloc_topology_init(&orig) == 0);
    assert(hwloc_topology_load(orig) == 0);

    int orig_depth = hwloc_topology_get_depth(orig);
    assert(orig_depth >= 1);
    int orig_pu_depth = hwloc_get_type_depth(orig, HWLOC_OBJ_PU);
    assert(orig_pu_depth != HWLOC_TYPE_DEPTH_UNKNOWN);
    unsigned int orig_nbpus = hwloc_get_nbobjs_by_depth(orig, orig_pu_depth);
    assert(orig_nbpus >= 1);

    /* Export to in-memory XML buffer. */
    char *xmlbuf = NULL;
    int xmlbuflen = 0;
    int rc = hwloc_topology_export_xmlbuffer(orig, &xmlbuf, &xmlbuflen, 0);
    assert(rc == 0);
    assert(xmlbuf != NULL);
    assert(xmlbuflen > 0);
    /* The export must be well-formed XML; sanity-check the header. */
    assert(strstr(xmlbuf, "<?xml") == xmlbuf);
    assert(strstr(xmlbuf, "topology") != NULL);

    /* Reimport into a fresh topology from the same buffer. */
    hwloc_topology_t reimported;
    assert(hwloc_topology_init(&reimported) == 0);
    assert(hwloc_topology_set_xmlbuffer(reimported, xmlbuf, xmlbuflen) == 0);
    assert(hwloc_topology_load(reimported) == 0);

    int reim_depth = hwloc_topology_get_depth(reimported);
    int reim_pu_depth = hwloc_get_type_depth(reimported, HWLOC_OBJ_PU);
    assert(reim_pu_depth != HWLOC_TYPE_DEPTH_UNKNOWN);
    unsigned int reim_nbpus = hwloc_get_nbobjs_by_depth(reimported, reim_pu_depth);

    assert(reim_depth == orig_depth);
    assert(reim_nbpus == orig_nbpus);

    hwloc_free_xmlbuffer(orig, xmlbuf);
    hwloc_topology_destroy(reimported);
    hwloc_topology_destroy(orig);

    printf("hwloc xml test OK: depth=%d, nbpus=%u, xmlbuflen=%d\n",
           orig_depth, orig_nbpus, xmlbuflen);
    return 0;
}
