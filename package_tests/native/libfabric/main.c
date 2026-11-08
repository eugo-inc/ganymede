/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libfabric: core API; calls fi_version(), fi_allocinfo(), fi_freeinfo(), fi_getinfo().
 */

#include <rdma/fabric.h>
#include <rdma/fi_errno.h>

#include <stdint.h>
#include <stdio.h>

int main(void) {
    const uint32_t version = fi_version();
    if (FI_MAJOR(version) != FI_MAJOR_VERSION) {
        fprintf(stderr, "header API major %d but the library reports %u.%u\n", FI_MAJOR_VERSION, FI_MAJOR(version),
                FI_MINOR(version));
        return 1;
    }

    struct fi_info *hints = fi_allocinfo();
    if (hints == NULL) {
        fprintf(stderr, "fi_allocinfo returned NULL\n");
        return 1;
    }
    fi_freeinfo(hints);

    struct fi_info *info = NULL;
    const int rc = fi_getinfo(FI_VERSION(FI_MAJOR_VERSION, FI_MINOR_VERSION), NULL, NULL, 0, NULL, &info);
    if (rc == 0) {
        printf("providers answering here:");
        for (const struct fi_info *entry = info; entry != NULL; entry = entry->next) {
            if (entry->fabric_attr != NULL && entry->fabric_attr->prov_name != NULL) {
                printf(" %s", entry->fabric_attr->prov_name);
            }
        }
        printf("\n");
        fi_freeinfo(info);
    } else {
        printf("fi_getinfo found no provider here (%s) - informational, no fabric hardware in this container\n",
               fi_strerror(-rc));
    }

    printf("libfabric %u.%u loaded\n", FI_MAJOR(version), FI_MINOR(version));
    return 0;
}
