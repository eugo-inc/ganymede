/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for unixodbc: dbc; calls SQLAllocHandle(), SQLSetEnvAttr(), SQLSetConnectAttr(), SQLFreeHandle().
 */

#include <sql.h>
#include <sqlext.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    SQLHENV env = SQL_NULL_HANDLE;
    SQLHDBC dbc = SQL_NULL_HANDLE;
    SQLRETURN rc;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(env != SQL_NULL_HANDLE);

    rc = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    rc = SQLAllocHandle(SQL_HANDLE_DBC, env, &dbc);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(dbc != SQL_NULL_HANDLE);

    /* SQL_ATTR_LOGIN_TIMEOUT is handled by the driver manager itself when no
     * driver is connected — exercises the connect-attr dispatch table without
     * needing a DSN. */
    rc = SQLSetConnectAttr(dbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)(SQLLEN)5, 0);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    rc = SQLFreeHandle(SQL_HANDLE_DBC, dbc);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc DBC handle test OK\n");
    return 0;
}
