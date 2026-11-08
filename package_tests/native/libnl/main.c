/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libnl: core API; calls nl_socket_alloc(), nl_socket_free(), rtnl_link_alloc(), rtnl_link_set_name().
 */

#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/socket.h>
#include <netlink/version.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    struct nl_sock *netlink_socket = nl_socket_alloc();
    if (netlink_socket == NULL) {
        fprintf(stderr, "nl_socket_alloc returned NULL\n");
        return 1;
    }
    nl_socket_free(netlink_socket);

    struct rtnl_link *link = rtnl_link_alloc();
    if (link == NULL) {
        fprintf(stderr, "rtnl_link_alloc returned NULL\n");
        return 1;
    }
    rtnl_link_set_name(link, "demo0");
    const char *name = rtnl_link_get_name(link);
    if (name == NULL || strcmp(name, "demo0") != 0) {
        fprintf(stderr, "rtnl_link_get_name returned %s\n", name != NULL ? name : "(null)");
        rtnl_link_put(link);
        return 1;
    }
    rtnl_link_put(link);

    if (nl_ver_num != LIBNL_VER_NUM) {
        fprintf(stderr, "version.h says 0x%x but the library says 0x%x\n", LIBNL_VER_NUM, nl_ver_num);
        return 1;
    }

    printf("%s: socket + route link objects OK\n", LIBNL_STRING);
    return 0;
}
