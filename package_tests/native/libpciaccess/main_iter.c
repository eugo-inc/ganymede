/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libpciaccess: iter; calls pci_system_init(), pci_id_match_iterator_create(), pci_device_next(), pci_iterator_destroy().
 */

#include <pciaccess.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int rc = pci_system_init();
    if (rc != 0) {
        printf("libpciaccess iter test OK: pci_system_init returned %d (no PCI in env)\n", rc);
        return 0;
    }

    struct pci_id_match match = {
        .vendor_id = PCI_MATCH_ANY,
        .device_id = PCI_MATCH_ANY,
        .subvendor_id = PCI_MATCH_ANY,
        .subdevice_id = PCI_MATCH_ANY,
        .device_class = 0,
        .device_class_mask = 0,
        .match_data = 0,
    };

    struct pci_device_iterator *iter = pci_id_match_iterator_create(&match);
    assert(iter != NULL);

    int count = 0;
    struct pci_device *dev;
    while ((dev = pci_device_next(iter)) != NULL) {
        count++;
        if (count > 4096) {
            /* Sanity: iterator should NOT return infinite devices. */
            fprintf(stderr, "libpciaccess iter test FAIL: iterator did not terminate\n");
            pci_iterator_destroy(iter);
            pci_system_cleanup();
            return 1;
        }
    }

    pci_iterator_destroy(iter);
    pci_system_cleanup();
    printf("libpciaccess iter test OK: walked %d device(s) via id-match iterator\n", count);
    return 0;
}
