/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for graphviz: layout; calls gvContext(), agmemread(), agnnodes(), agnedges().
 */

#include <graphviz/gvc.h>
#include <graphviz/cgraph.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    GVC_t *gvc = gvContext();
    assert(gvc != NULL);

    const char *dot_src =
        "digraph G { rankdir=LR; A -> B; B -> C; A -> C; }";
    Agraph_t *g = agmemread(dot_src);
    assert(g != NULL);

    /* Verify parsed graph topology. */
    assert(agnnodes(g) == 3);
    assert(agnedges(g) == 3);

    int node_count = 0;
    for (Agnode_t *n = agfstnode(g); n != NULL; n = agnxtnode(g, n)) {
        const char *name = agnameof(n);
        assert(name != NULL);
        assert(strlen(name) == 1);
        assert(name[0] == 'A' || name[0] == 'B' || name[0] == 'C');
        node_count++;
    }
    assert(node_count == 3);

    int edge_count = 0;
    for (Agnode_t *n = agfstnode(g); n != NULL; n = agnxtnode(g, n)) {
        for (Agedge_t *e = agfstout(g, n); e != NULL; e = agnxtout(g, e)) {
            assert(agtail(e) != NULL);
            assert(aghead(e) != NULL);
            edge_count++;
        }
    }
    assert(edge_count == 3);

    /* Verify the graph reports itself as directed (matches "digraph"). */
    assert(agisdirected(g) != 0);

    agclose(g);
    int fini_rc = gvFreeContext(gvc);
    assert(fini_rc == 0);

    printf("graphviz gvc+cgraph context/parse test OK\n");
    return 0;
}
