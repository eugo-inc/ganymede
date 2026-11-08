/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for unixodbc: odbcinst; calls SQLGetPrivateProfileString(), SQLInstallerError().
 */

#include <odbcinst.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    char buf[256] = {0};
    const char *default_value = "DEMO_DEFAULT";

    int n = SQLGetPrivateProfileString(
        "DemoNonexistentSection", "DemoNonexistentKey", default_value,
        buf, (int)sizeof(buf), "odbcinst.ini"
    );
    assert(n >= 0);
    /* When no entry exists, the default value is copied verbatim. */
    assert(strcmp(buf, default_value) == 0);

    DWORD err_code = 0;
    char err_msg[256] = {0};
    WORD err_msg_len = 0;
    RETCODE installer_rc = SQLInstallerError(
        1, &err_code, err_msg, (WORD)sizeof(err_msg), &err_msg_len
    );
    /* Acceptable returns: SQL_NO_DATA (no error pending) or SQL_SUCCESS
     * (an error was logged during ProfileString lookup). Both are valid. */
    assert(installer_rc == SQL_NO_DATA
           || installer_rc == SQL_SUCCESS
           || installer_rc == SQL_SUCCESS_WITH_INFO);

    printf("unixodbc odbcinst test OK (default-fallback length=%d)\n", n);
    return 0;
}
