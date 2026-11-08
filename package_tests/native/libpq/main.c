/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpq: core API; calls PQlibVersion(), PQconninfoParse(), PQconninfoFree(), PQescapeString().
 */

#include <libpq-fe.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    int version = PQlibVersion();
    assert(version >= 100000);

    /* Parse a representative conninfo string. PQconninfoParse never touches the network. */
    char *errmsg = NULL;
    PQconninfoOption *opts = PQconninfoParse(
        "host=localhost dbname=postgres user=demo", &errmsg);
    assert(opts != NULL);
    assert(errmsg == NULL);

    int saw_host = 0, saw_dbname = 0;
    for (PQconninfoOption *o = opts; o->keyword != NULL; ++o) {
        if (strcmp(o->keyword, "host") == 0 && o->val && strcmp(o->val, "localhost") == 0)
            saw_host = 1;
        if (strcmp(o->keyword, "dbname") == 0 && o->val && strcmp(o->val, "postgres") == 0)
            saw_dbname = 1;
    }
    assert(saw_host);
    assert(saw_dbname);
    PQconninfoFree(opts);

    /* Client-side string escape: O'Reilly -> O''Reilly. */
    const char *input = "O'Reilly";
    char escaped[32];
    size_t n = PQescapeString(escaped, input, strlen(input));
    assert(n == strlen("O''Reilly"));
    assert(strcmp(escaped, "O''Reilly") == 0);

    printf("libpq test OK: version=%d\n", version);
    return 0;
}
