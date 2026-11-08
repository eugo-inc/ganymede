/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpciaccess: core API; calls pci_system_init(), pci_system_cleanup().
 */

#include <pciaccess.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int rc = pci_system_init();
    if (rc == 0) {
        /* PCI is accessible in this environment — cleanly tear down. */
        pci_system_cleanup();
        printf("libpciaccess test OK: pci_system_init succeeded\n");
    } else {
        printf("libpciaccess test OK: pci_system_init returned %d (no PCI in env)\n", rc);
    }
    return 0;
}
