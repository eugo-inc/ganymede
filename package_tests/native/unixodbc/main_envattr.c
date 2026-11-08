/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for unixodbc: envattr; calls SQLAllocHandle(), SQLSetEnvAttr(), SQLGetEnvAttr(), SQLFreeHandle().
 */

#include <sql.h>
#include <sqlext.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    SQLHENV env = SQL_NULL_HANDLE;
    SQLRETURN rc;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(env != SQL_NULL_HANDLE);

    rc = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    /* Read the attribute back. SQLGetEnvAttr writes an SQLINTEGER-sized value
     * (the ODBC version constant) into the provided buffer. */
    SQLINTEGER version_out = 0;
    SQLINTEGER actual_len = 0;
    rc = SQLGetEnvAttr(env, SQL_ATTR_ODBC_VERSION,
                       &version_out, (SQLINTEGER)sizeof(version_out),
                       &actual_len);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(version_out == SQL_OV_ODBC3);

    /* Read a second, distinct attribute to exercise the dispatch table rather
     * than a single hard-coded case. SQL_ATTR_CONNECTION_POOLING defaults to
     * SQL_CP_OFF and is queryable without setting it first. */
    SQLINTEGER pooling_out = -1;
    rc = SQLGetEnvAttr(env, SQL_ATTR_CONNECTION_POOLING,
                       &pooling_out, (SQLINTEGER)sizeof(pooling_out),
                       NULL);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(pooling_out >= 0);

    rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc env-attr round-trip OK (version=%ld, pooling=%ld)\n",
           (long)version_out, (long)pooling_out);
    return 0;
}
