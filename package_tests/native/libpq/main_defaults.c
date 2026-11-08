/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpq: defaults; calls PQconndefaults(), PQconninfoFree().
 */

#include <libpq-fe.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    PQconninfoOption *defaults = PQconndefaults();
    assert(defaults != NULL);

    /* Walk the array and count well-known entries. The libpq client connection
     * vocabulary always includes at least these keywords. */
    int saw_host = 0, saw_port = 0, saw_dbname = 0, saw_user = 0;
    int total = 0;
    for (PQconninfoOption *o = defaults; o->keyword != NULL; ++o) {
        ++total;
        assert(o->label != NULL);
        /* dispsize is documented as the field width in characters; should be sane. */
        assert(o->dispsize >= 0);

        if (strcmp(o->keyword, "host") == 0)   saw_host = 1;
        if (strcmp(o->keyword, "port") == 0)   saw_port = 1;
        if (strcmp(o->keyword, "dbname") == 0) saw_dbname = 1;
        if (strcmp(o->keyword, "user") == 0)   saw_user = 1;
    }

    /* A working libpq exposes at least these four canonical keywords. */
    assert(saw_host);
    assert(saw_port);
    assert(saw_dbname);
    assert(saw_user);
    assert(total >= 10);

    PQconninfoFree(defaults);

    printf("libpq PQconndefaults OK: %d options\n", total);
    return 0;
}
