/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for unixodbc: drivers; calls SQLAllocHandle(), SQLSetEnvAttr(), SQLDrivers(), SQLFreeHandle().
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

    SQLCHAR driver_desc[256] = {0};
    SQLSMALLINT driver_desc_len = -1;
    SQLCHAR driver_attr[1024] = {0};
    SQLSMALLINT driver_attr_len = -1;

    SQLRETURN drv_rc = SQLDrivers(
        env, SQL_FETCH_FIRST,
        driver_desc, (SQLSMALLINT)sizeof(driver_desc), &driver_desc_len,
        driver_attr, (SQLSMALLINT)sizeof(driver_attr), &driver_attr_len
    );

    assert(drv_rc == SQL_SUCCESS
           || drv_rc == SQL_SUCCESS_WITH_INFO
           || drv_rc == SQL_NO_DATA);

    /* Length outputs should have been touched (set to >= 0). The driver
     * manager initializes them even on SQL_NO_DATA. */
    assert(driver_desc_len >= 0);
    assert(driver_attr_len >= 0);

    rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc SQLDrivers test OK (rc=%d, desc_len=%d, attr_len=%d)\n",
           (int)drv_rc, (int)driver_desc_len, (int)driver_attr_len);
    return 0;
}
