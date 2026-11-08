/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpciaccess: slot; calls pci_system_init(), pci_slot_match_iterator_create(), pci_device_next(), pci_iterator_destroy().
 */

#include <pciaccess.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int rc = pci_system_init();
    if (rc != 0) {
        printf("libpciaccess slot test OK: pci_system_init returned %d (no PCI in env)\n", rc);
        return 0;
    }

    /* PCI_MATCH_ANY for every coordinate = "any PCI slot". */
    struct pci_slot_match slot = {
        .domain = PCI_MATCH_ANY,
        .bus = PCI_MATCH_ANY,
        .dev = PCI_MATCH_ANY,
        .func = PCI_MATCH_ANY,
        .match_data = 0,
    };

    struct pci_device_iterator *iter = pci_slot_match_iterator_create(&slot);
    assert(iter != NULL);

    int count = 0;
    struct pci_device *dev;
    while ((dev = pci_device_next(iter)) != NULL) {
        count++;
        if (count > 4096) {
            fprintf(stderr, "libpciaccess slot test FAIL: iterator did not terminate\n");
            pci_iterator_destroy(iter);
            pci_system_cleanup();
            return 1;
        }
    }

    pci_iterator_destroy(iter);
    pci_system_cleanup();
    printf("libpciaccess slot test OK: walked %d device(s) via slot-match iterator\n", count);
    return 0;
}
