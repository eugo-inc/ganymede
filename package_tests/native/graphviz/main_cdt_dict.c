/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for graphviz: cdt dict; calls dtopen(), dtinsert(), dtmatch(), dtfirst().
 */

#include <graphviz/cdt.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    Dtlink_t link;
    int key;
    int value;
} Entry;

static void *make_entry(void *obj, Dtdisc_t *disc) {
    (void)disc;
    Entry *src = (Entry *)obj;
    Entry *copy = (Entry *)malloc(sizeof(Entry));
    assert(copy != NULL);
    copy->key = src->key;
    copy->value = src->value;
    return copy;
}

static void free_entry(void *obj) {
    free(obj);
}

static int cmp_entry(void *a, void *b) {
    int ka = *(int *)a;
    int kb = *(int *)b;
    return (ka < kb) ? -1 : (ka > kb) ? 1 : 0;
}

int main(void) {
    Dtdisc_t disc;
    memset(&disc, 0, sizeof(disc));
    disc.key = offsetof(Entry, key);
    disc.size = sizeof(int);
    disc.link = offsetof(Entry, link);
    disc.makef = make_entry;
    disc.freef = free_entry;
    disc.comparf = cmp_entry;

    /* Dtoset = ordered set; supports in-order walk + binary lookup. */
    Dt_t *dt = dtopen(&disc, Dtoset);
    assert(dt != NULL);

    Entry e1 = { .key = 30, .value = 300 };
    Entry e2 = { .key = 10, .value = 100 };
    Entry e3 = { .key = 20, .value = 200 };
    assert(dtinsert(dt, &e1) != NULL);
    assert(dtinsert(dt, &e2) != NULL);
    assert(dtinsert(dt, &e3) != NULL);

    /* Lookup by key. */
    int probe_key = 20;
    Entry *found = (Entry *)dtmatch(dt, &probe_key);
    assert(found != NULL);
    assert(found->value == 200);

    /* Lookup of a missing key returns NULL. */
    int missing_key = 999;
    assert(dtmatch(dt, &missing_key) == NULL);

    /* Walk the ordered set in ascending key order. */
    int expected_keys[3] = { 10, 20, 30 };
    int i = 0;
    for (Entry *cur = (Entry *)dtfirst(dt);
         cur != NULL;
         cur = (Entry *)dtnext(dt, cur)) {
        assert(i < 3);
        assert(cur->key == expected_keys[i]);
        i++;
    }
    assert(i == 3);

    /* Delete the middle entry; size drops to 2; key no longer matches. */
    int del_key = 20;
    Entry *to_del = (Entry *)dtmatch(dt, &del_key);
    assert(to_del != NULL);
    assert(dtdelete(dt, to_del) != NULL);
    assert(dtmatch(dt, &del_key) == NULL);

    int remaining = 0;
    for (Entry *cur = (Entry *)dtfirst(dt);
         cur != NULL;
         cur = (Entry *)dtnext(dt, cur)) {
        remaining++;
    }
    assert(remaining == 2);

    dtclose(dt);

    printf("graphviz cdt dict test OK\n");
    return 0;
}
