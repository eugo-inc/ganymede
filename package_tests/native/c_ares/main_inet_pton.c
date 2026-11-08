/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for c_ares: inet pton; calls ares_inet_pton(), ares_inet_ntop().
 */

#include <ares.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    struct in_addr v4 = {0};
    int rc = ares_inet_pton(AF_INET, "8.8.8.8", &v4);
    assert(rc == 1);

    char v4_buf[INET_ADDRSTRLEN] = {0};
    const char *v4_back = ares_inet_ntop(AF_INET, &v4, v4_buf, sizeof(v4_buf));
    assert(v4_back != NULL);
    assert(strcmp(v4_back, "8.8.8.8") == 0);

    struct in6_addr v6;
    memset(&v6, 0, sizeof(v6));
    rc = ares_inet_pton(AF_INET6, "2001:4860:4860::8888", &v6);
    assert(rc == 1);

    char v6_buf[INET6_ADDRSTRLEN] = {0};
    const char *v6_back = ares_inet_ntop(AF_INET6, &v6, v6_buf, sizeof(v6_buf));
    assert(v6_back != NULL);
    /* ntop output is canonical lowercase / shortest form. */
    assert(strcmp(v6_back, "2001:4860:4860::8888") == 0);

    /* --- Garbage input rejected for IPv4 --- */
    struct in_addr garbage = {0};
    rc = ares_inet_pton(AF_INET, "not.an.ip", &garbage);
    assert(rc <= 0);   /* 0 = unparseable, -1 = error; both are non-success. */

    /* --- Garbage input rejected for IPv6 --- */
    struct in6_addr garbage6;
    memset(&garbage6, 0, sizeof(garbage6));
    rc = ares_inet_pton(AF_INET6, "definitely::not::a::v6", &garbage6);
    assert(rc <= 0);

    /* --- Invalid address family rejected --- */
    struct in_addr ignored = {0};
    rc = ares_inet_pton(/* bogus af */ 0xBEEF, "8.8.8.8", &ignored);
    assert(rc <= 0);

    printf("c_ares inet_pton test OK: v4=%s v6=%s\n", v4_back, v6_back);
    return 0;
}
