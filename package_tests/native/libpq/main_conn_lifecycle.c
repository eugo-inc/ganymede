/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpq: conn lifecycle; calls PQconnectdb(), PQstatus(), PQerrorMessage(), PQdb().
 */

#include <libpq-fe.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *conninfo =
        "host=demo-libpq-test.invalid hostaddr=192.0.2.1 port=1 "
        "dbname=postgres user=demo connect_timeout=1";

    PGconn *conn = PQconnectdb(conninfo);
    assert(conn != NULL);

    ConnStatusType status = PQstatus(conn);
    assert(status == CONNECTION_BAD);

    const char *err = PQerrorMessage(conn);
    assert(err != NULL);
    const size_t err_len = strlen(err);
    assert(err_len > 0);

    const char *db = PQdb(conn);
    assert(db != NULL);
    assert(strcmp(db, "postgres") == 0);

    const char *user = PQuser(conn);
    assert(user != NULL);
    assert(strcmp(user, "demo") == 0);

    /* Backend PID should be 0 on a failed connection. */
    assert(PQbackendPID(conn) == 0);

    PQfinish(conn);

    /* PQping should also report failure without hanging. */
    PGPing ping = PQping(conninfo);
    /* Any of these is acceptable for an unreachable host: no response, rejection, no attempt. */
    assert(ping == PQPING_NO_RESPONSE
        || ping == PQPING_REJECT
        || ping == PQPING_NO_ATTEMPT);

    /* err_len, NOT strlen(err) — `err` points into the PGconn freed by PQfinish above. */
    printf("libpq lifecycle OK: status=%d ping=%d errlen=%zu\n",
           (int)status, (int)ping, err_len);
    return 0;
}
