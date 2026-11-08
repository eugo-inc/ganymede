/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for elfutils: libdw; calls dwarf_begin(), dwarf_errmsg(), dwarf_nextcu(), dwarf_offdie().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <elfutils/libdw.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    int fd = open("/proc/self/exe", O_RDONLY);
    assert(fd >= 0);

    Dwarf *dbg = dwarf_begin(fd, DWARF_C_READ);

    if (dbg == NULL) {
        /* testspec.json builds this probe with -g, so DWARF is present and a
         * NULL means libdw could not read it. dwarf_errmsg(-1) names the last
         * libdw error. */
        fprintf(stderr, "FAIL: dwarf_begin returned NULL on a binary built with -g: %s\n", dwarf_errmsg(-1));
        close(fd);
        return 1;
    }

    /* Walk the compilation-unit list. dwarf_nextcu returns 0 on success, 1 at
     * EOF, -1 on error. */
    Dwarf_Off off = 0;
    Dwarf_Off next_off = 0;
    size_t header_size = 0;
    size_t cu_count = 0;
    int rc;
    while ((rc = dwarf_nextcu(dbg, off, &next_off, &header_size,
                              NULL, NULL, NULL)) == 0) {
        Dwarf_Die cu_die;
        /* The CU DIE sits right after the CU header. */
        Dwarf_Die *got = dwarf_offdie(dbg, off + header_size, &cu_die);
        assert(got == &cu_die);

        /* A compilation-unit DIE must carry tag DW_TAG_compile_unit (0x11)
         * or DW_TAG_partial_unit (0x3c) or DW_TAG_skeleton_unit (0x4a). */
        int tag = dwarf_tag(&cu_die);
        assert(tag == 0x11 || tag == 0x3c || tag == 0x4a);

        ++cu_count;
        off = next_off;
    }
    assert(rc == 1); /* clean EOF, not an error */
    assert(cu_count > 0);

    assert(dwarf_end(dbg) == 0);
    close(fd);

    printf("elfutils libdw CU-walk OK (%zu compilation units)\n", cu_count);
    return 0;
}
