/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for gdrcopy: core API; calls gdr_runtime_get_version(), gdr_open(), gdr_close().
 */

#include <dlfcn.h>
#include <gdrapi.h>
#include <stdio.h>

int main(void) {
    int major = 0, minor = 0;
    gdr_runtime_get_version(&major, &minor);
    printf("gdrcopy: library %d.%d, header %d.%d\n", major, minor, GDR_API_MAJOR_VERSION, GDR_API_MINOR_VERSION);
    if (major != GDR_API_MAJOR_VERSION) {
        fprintf(stderr, "gdrcopy: library major %d differs from the header's %d\n", major, GDR_API_MAJOR_VERSION);
        return 1;
    }

    void *handle = dlopen("libgdrapi.so.2", RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        fprintf(stderr, "gdrcopy: dlopen(libgdrapi.so.2) failed: %s\n", dlerror());
        return 1;
    }
    /* Entry points of the GDRCopy 2.5+ API (gdr_pin_buffer_v2, gdr_map_v2 and gdr_get_attribute are not in 2.4). */
    const char *symbols[] = {"gdr_open", "gdr_close", "gdr_pin_buffer", "gdr_unpin_buffer", "gdr_map", "gdr_unmap",
                             "gdr_get_info", "gdr_get_info_v2", "gdr_copy_to_mapping", "gdr_copy_from_mapping",
                             "gdr_runtime_get_version", "gdr_driver_get_version", "gdr_pin_buffer_v2", "gdr_map_v2",
                             "gdr_get_attribute", NULL};
    for (int i = 0; symbols[i]; i++) {
        if (!dlsym(handle, symbols[i])) {
            fprintf(stderr, "gdrcopy: %s is missing from libgdrapi.so.2\n", symbols[i]);
            return 1;
        }
    }

    gdr_t g = gdr_open();
    if (g) {
        printf("gdrcopy: gdr_open() opened a GPU mapping backend on this machine\n");
        gdr_close(g);
    } else {
        printf("gdrcopy: gdr_open() found no backend here (no gdrdrv module and no DMA-BUF support); this is not an error\n");
    }
    return 0;
}
