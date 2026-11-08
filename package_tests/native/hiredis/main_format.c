/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hiredis: format; calls redisFormatCommand(), redisFreeCommand().
 */

#include <hiredis/hiredis.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char expected[] =
        "*3\r\n"
        "$3\r\nSET\r\n"
        "$5\r\nmykey\r\n"
        "$5\r\nmyval\r\n";
    const int expected_len = (int)(sizeof(expected) - 1);

    char *cmd = NULL;
    int len = redisFormatCommand(&cmd, "SET %s %s", "mykey", "myval");
    assert(len > 0);
    assert(cmd != NULL);
    assert(len == expected_len);
    assert(memcmp(cmd, expected, (size_t)len) == 0);
    redisFreeCommand(cmd);
    cmd = NULL;

    /* Verify %b (binary-safe) handles embedded NULs that %s would truncate. */
    static const char bin_payload[] = {'a', '\0', 'b', '\0', 'c'};
    len = redisFormatCommand(&cmd, "SET %s %b", "k", bin_payload,
                             (size_t)sizeof(bin_payload));
    assert(len > 0);
    assert(cmd != NULL);
    /* The bulk-string header for the 5-byte binary value must read "$5\r\n",
     * proving the length came from the size_t arg and not strlen(). */
    assert(strstr(cmd, "$5\r\n") != NULL);
    const char *payload_start = strstr(cmd, "$5\r\n");
    assert(payload_start != NULL);
    payload_start += 4;  /* skip "$5\r\n" */
    assert(memcmp(payload_start, bin_payload, sizeof(bin_payload)) == 0);
    redisFreeCommand(cmd);

    printf("hiredis format test OK\n");
    return 0;
}
