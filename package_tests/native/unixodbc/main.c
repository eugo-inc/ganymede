/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for unixodbc: core API; calls SQLAllocHandle(), SQLSetEnvAttr(), SQLFreeHandle().
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

    rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc test OK\n");
    return 0;
}
