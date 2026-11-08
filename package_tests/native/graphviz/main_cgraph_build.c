/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for graphviz: cgraph build; calls agopen(), agnode(), agnnodes(), agedge().
 */

#include <graphviz/cgraph.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Create a strict directed graph (Agstrictdirected => no parallel edges). */
    Agraph_t *g = agopen("probe", Agstrictdirected, NULL);
    assert(g != NULL);

    Agnode_t *n1 = agnode(g, "alpha", 1);
    Agnode_t *n2 = agnode(g, "beta", 1);
    Agnode_t *n3 = agnode(g, "gamma", 1);
    assert(n1 != NULL && n2 != NULL && n3 != NULL);
    assert(agnnodes(g) == 3);

    /* Looking up an existing node (create=0) returns the same handle. */
    Agnode_t *n1_again = agnode(g, "alpha", 0);
    assert(n1_again == n1);

    /* Looking up a missing node with create=0 returns NULL. */
    Agnode_t *missing = agnode(g, "does_not_exist", 0);
    assert(missing == NULL);

    /* Add edges; strict directed => duplicate (n1,n2) edge is collapsed.
     * Note: cgraph may return distinct Agedge_t* handles for the same logical
     * edge (e.g. in-edge vs out-edge views), so compare by AGID (the stable
     * object id) rather than pointer equality. The edge count is the
     * authoritative check that no parallel edge was actually created. */
    Agedge_t *e12 = agedge(g, n1, n2, NULL, 1);
    Agedge_t *e23 = agedge(g, n2, n3, NULL, 1);
    Agedge_t *e12_dup = agedge(g, n1, n2, NULL, 1);
    assert(e12 != NULL && e23 != NULL && e12_dup != NULL);
    assert(AGID(e12_dup) == AGID(e12));
    assert(agnedges(g) == 2);

    Agsym_t *sym = agattr(g, AGNODE, "label", "");
    assert(sym != NULL);

    int set_rc = agset(n1, "label", "Alpha Node");
    assert(set_rc == 0);
    const char *got = agget(n1, "label");
    assert(got != NULL);
    assert(strcmp(got, "Alpha Node") == 0);

    const char *default_got = agget(n2, "label");
    assert(default_got != NULL);
    assert(strcmp(default_got, "") == 0);

    /* Walk all nodes and count: must equal agnnodes. */
    int walked = 0;
    for (Agnode_t *n = agfstnode(g); n != NULL; n = agnxtnode(g, n)) {
        walked++;
    }
    assert(walked == 3);

    agclose(g);

    printf("graphviz cgraph build test OK\n");
    return 0;
}
