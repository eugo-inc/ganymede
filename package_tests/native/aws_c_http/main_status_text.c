/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_c_http: status text; calls aws_http_status_text().
 */

#include <aws/http/http.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *ok = aws_http_status_text(200);
    assert(ok != NULL);
    assert(strcmp(ok, "OK") == 0);

    const char *not_found = aws_http_status_text(404);
    assert(not_found != NULL);
    assert(strcmp(not_found, "Not Found") == 0);

    const char *server_err = aws_http_status_text(500);
    assert(server_err != NULL);
    assert(strcmp(server_err, "Internal Server Error") == 0);

    /* Unknown codes return empty string, NOT NULL — see http.h doc comment. */
    const char *unknown = aws_http_status_text(999);
    assert(unknown != NULL);
    assert(unknown[0] == '\0');

    printf("aws_c_http status_text test OK\n");
    return 0;
}
