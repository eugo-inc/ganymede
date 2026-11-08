/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for rdma_core: device list; calls ibv_get_sysfs_path(), ibv_get_device_list(), ibv_get_device_name(), ibv_free_device_list().
 */

#include <infiniband/verbs.h>

#include <linux/netlink.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

const char *ibv_get_sysfs_path(void);

/* The netlink half of what libibverbs tries first (util/rdma_nl.c: a NETLINK_RDMA socket, connected by libnl). A kernel
 * without ib_core refuses the socket with EPROTONOSUPPORT. */
static int rdma_netlink_is_offered(void) {
    int socket_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_RDMA);
    if (socket_fd < 0) {
        return 0;
    }
    struct sockaddr_nl address = {.nl_family = AF_NETLINK};
    int is_bound = bind(socket_fd, (struct sockaddr *)&address, sizeof(address)) == 0;
    close(socket_fd);
    return is_bound;
}

/* The sysfs half: the directory libibverbs falls back to (init.c, find_sysfs_devs). */
static int uverbs_sysfs_class_is_offered(void) {
    const char *sysfs_root = ibv_get_sysfs_path();
    char class_path[4096];
    struct stat class_stat;
    if (sysfs_root == NULL) {
        return 0;
    }
    snprintf(class_path, sizeof(class_path), "%s/class/infiniband_verbs", sysfs_root);
    return stat(class_path, &class_stat) == 0 && S_ISDIR(class_stat.st_mode);
}

int main(void) {
    int device_count = 0;
    errno = 0;
    struct ibv_device **devices = ibv_get_device_list(&device_count);
    int list_errno = errno;

    if (devices == NULL) {
        fprintf(stderr,
                "ibv_get_device_list returned NULL (%s) on a host the runner found to offer RDMA; this program sees "
                "netlink: %s, %s/class/infiniband_verbs: %s\n",
                strerror(list_errno), rdma_netlink_is_offered() ? "yes" : "no",
                ibv_get_sysfs_path() != NULL ? ibv_get_sysfs_path() : "<sysfs>",
                uverbs_sysfs_class_is_offered() ? "yes" : "no");
        return 1;
    }

    printf("%d RDMA device(s):", device_count);
    for (int i = 0; i < device_count; ++i) {
        printf(" %s", ibv_get_device_name(devices[i]));
    }
    printf("\n");
    ibv_free_device_list(devices);
    return 0;
}
