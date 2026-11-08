/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for elfutils: libelf; calls elf_version(), elf_begin(), elf_kind(), elf_getshdrnum().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    assert(elf_version(EV_CURRENT) != EV_NONE);

    int fd = open("/proc/self/exe", O_RDONLY);
    assert(fd >= 0);

    Elf *e = elf_begin(fd, ELF_C_READ, NULL);
    assert(e != NULL);

    /* The file should classify as a regular ELF object (ELF_K_ELF), not an
     * archive (ELF_K_AR) and not raw data (ELF_K_NONE). */
    assert(elf_kind(e) == ELF_K_ELF);

    /* A real executable must have at least one section. */
    size_t nshdr = 0;
    assert(elf_getshdrnum(e, &nshdr) == 0);
    assert(nshdr > 0);

    /* It must also expose a section-header string-table index — the
     * mechanism libelf uses to resolve section names. */
    size_t shstrndx = 0;
    assert(elf_getshdrstrndx(e, &shstrndx) == 0);
    assert(shstrndx < nshdr);

    /* Walk every section via the elf_nextscn iterator; each must yield a
     * valid GElf_Shdr through gelf_getshdr (the class-independent accessor). */
    size_t walked = 0;
    Elf_Scn *scn = NULL;
    while ((scn = elf_nextscn(e, scn)) != NULL) {
        GElf_Shdr shdr;
        assert(gelf_getshdr(scn, &shdr) == &shdr);
        ++walked;
    }
    /* nshdr counts the NULL section at index 0; nextscn skips it, so
     * walked == nshdr - 1. */
    assert(walked + 1 == nshdr);

    assert(elf_end(e) == 0);
    close(fd);

    printf("elfutils libelf section-walk on /proc/self/exe OK (%zu sections)\n",
           nshdr);
    return 0;
}
