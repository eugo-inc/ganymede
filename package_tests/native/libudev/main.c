/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libudev: core API; calls udev_enumerate_new(), udev_enumerate_add_match_subsystem(), udev_enumerate_scan_devices(), udev_enumerate_get_list_entry().
 */

#include <dlfcn.h>
#include <libudev.h>
#include <stdio.h>

static int count(struct udev *udev, const char *subsystem) {
    struct udev_enumerate *e = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(e, subsystem);
    udev_enumerate_scan_devices(e);
    int n = 0;
    struct udev_list_entry *it;
    udev_list_entry_foreach(it, udev_enumerate_get_list_entry(e)) {
        struct udev_device *d = udev_device_new_from_syspath(udev, udev_list_entry_get_name(it));
        if (d && udev_device_get_subsystem(d)) n++;
        if (d) udev_device_unref(d);
    }
    udev_enumerate_unref(e);
    return n;
}

int main(void) {
    struct udev *udev = udev_new();
    if (!udev) {
        fprintf(stderr, "FAIL udev_new returned NULL\n");
        return 1;
    }
    int pci = count(udev, "pci");
    int block = count(udev, "block");
    udev_unref(udev);
    void *handle = dlopen("libudev.so", RTLD_NOW);
    if (!handle || !dlsym(handle, "udev_new")) {
        fprintf(stderr, "FAIL dlopen(\"libudev.so\"): %s\n", dlerror());
        return 2;
    }
    printf("OK libudev: %d PCI and %d block device(s) read from sysfs; dlopen(\"libudev.so\") works\n", pci, block);
    return 0;
}
