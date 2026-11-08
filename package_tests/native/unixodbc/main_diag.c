/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for unixodbc: diag; calls SQLAllocHandle(), SQLSetEnvAttr(), SQLGetDiagRec(), SQLFreeHandle().
 */

#include <sql.h>
#include <sqlext.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    SQLHENV env = SQL_NULL_HANDLE;
    SQLRETURN rc;

    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);
    assert(env != SQL_NULL_HANDLE);

    /* Trigger an error: passing -1 as the attribute identifier is invalid. The
     * driver manager must reject it with SQL_ERROR and populate a diag record. */
    rc = SQLSetEnvAttr(env, (SQLINTEGER)-1, (SQLPOINTER)SQL_OV_ODBC3, 0);
    assert(rc == SQL_ERROR);

    SQLCHAR sqlstate[6] = {0};
    SQLINTEGER native_error = 0;
    SQLCHAR message[256] = {0};
    SQLSMALLINT message_len = 0;

    SQLRETURN diag_rc = SQLGetDiagRec(
        SQL_HANDLE_ENV, env, 1,
        sqlstate, &native_error,
        message, (SQLSMALLINT)sizeof(message), &message_len
    );
    assert(diag_rc == SQL_SUCCESS || diag_rc == SQL_SUCCESS_WITH_INFO);

    /* ODBC SQLSTATE is always exactly 5 characters (plus NUL). */
    assert(strlen((const char *)sqlstate) == 5);
    /* The message buffer should have received some text. */
    assert(message_len > 0);
    assert(message[0] != '\0');

    rc = SQLFreeHandle(SQL_HANDLE_ENV, env);
    assert(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc diag test OK (SQLSTATE=%s)\n", (const char *)sqlstate);
    return 0;
}
