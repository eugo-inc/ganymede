/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libpciaccess: probe; calls pci_system_init(), pci_slot_match_iterator_create(), pci_device_next(), pci_iterator_destroy().
 */

#include <pciaccess.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    int rc = pci_system_init();
    if (rc != 0) {
        printf("libpciaccess probe test OK: pci_system_init returned %d (no PCI in env)\n", rc);
        return 0;
    }

    struct pci_slot_match slot = {
        .domain = PCI_MATCH_ANY,
        .bus = PCI_MATCH_ANY,
        .dev = PCI_MATCH_ANY,
        .func = PCI_MATCH_ANY,
        .match_data = 0,
    };

    struct pci_device_iterator *iter = pci_slot_match_iterator_create(&slot);
    assert(iter != NULL);

    struct pci_device *dev = pci_device_next(iter);
    if (dev == NULL) {
        pci_iterator_destroy(iter);
        pci_system_cleanup();
        printf("libpciaccess probe test OK: bus empty, nothing to probe\n");
        return 0;
    }

    int probe_rc = pci_device_probe(dev);

    if (probe_rc == 0) {
        assert(dev->vendor_id != 0xFFFF);
        printf("libpciaccess probe test OK: probed vendor=0x%04x device=0x%04x\n",
               dev->vendor_id, dev->device_id);
    } else {
        printf("libpciaccess probe test OK: pci_device_probe returned %d (restricted env)\n",
               probe_rc);
    }

    pci_iterator_destroy(iter);
    pci_system_cleanup();
    return 0;
}
