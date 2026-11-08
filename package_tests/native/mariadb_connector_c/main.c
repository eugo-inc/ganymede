/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for mariadb_connector_c: core API; calls mysql_library_init(), mysql_init(), mysql_get_client_info(), mysql_get_client_version().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <mariadb/mysql.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* mysql_library_init: 0 == success. */
    assert(mysql_library_init(0, NULL, NULL) == 0);

    MYSQL *h = mysql_init(NULL);
    assert(h != NULL);

    /* Client info / version are part of the documented contract; available without a connection. */
    const char *client_info = mysql_get_client_info();
    assert(client_info != NULL);
    assert(strlen(client_info) > 0);

    unsigned long client_version = mysql_get_client_version();
    assert(client_version > 0);

    mysql_close(h);
    mysql_library_end();

    printf("mariadb_connector_c test OK: %s\n", client_info);
    return 0;
}
