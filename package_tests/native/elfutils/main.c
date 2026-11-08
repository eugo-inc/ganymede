/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for elfutils: core API; calls elf_version().
 */

#include <libelf.h>
#include <stdio.h>

int main(void) {
    unsigned ver = elf_version(EV_CURRENT);
    if (ver != EV_CURRENT) {
        fprintf(stderr, "FAIL: elf_version(EV_CURRENT) returned %u, expected %u\n", ver, (unsigned)EV_CURRENT);
        return 1;
    }
    /* An ELF version libelf does not know must be refused with EV_NONE. */
    unsigned bad = elf_version(EV_NUM + 7u);
    if (bad != EV_NONE) {
        fprintf(stderr, "FAIL: elf_version(EV_NUM + 7) returned %u, expected EV_NONE\n", bad);
        return 1;
    }
    printf("elfutils test OK\n");
    return 0;
}
