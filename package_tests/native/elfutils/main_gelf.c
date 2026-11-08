/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for elfutils: gelf; calls elf_version(), elf_begin(), elf_kind(), gelf_getclass().
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
    assert(elf_kind(e) == ELF_K_ELF);

    int cls = gelf_getclass(e);
    assert(cls == ELFCLASS32 || cls == ELFCLASS64);

    /* Pull the ELF header into the class-independent GElf_Ehdr. */
    GElf_Ehdr ehdr;
    assert(gelf_getehdr(e, &ehdr) == &ehdr);

    /* The 0x7F 'E' 'L' 'F' magic must be present. */
    assert(ehdr.e_ident[EI_MAG0] == ELFMAG0);
    assert(ehdr.e_ident[EI_MAG1] == ELFMAG1);
    assert(ehdr.e_ident[EI_MAG2] == ELFMAG2);
    assert(ehdr.e_ident[EI_MAG3] == ELFMAG3);

    /* Class byte must agree with gelf_getclass. */
    assert(ehdr.e_ident[EI_CLASS] == cls);

    /* An executable test program is either ET_EXEC (statically positioned)
     * or, more commonly under modern PIE defaults, ET_DYN. Anything else
     * (ET_REL / ET_CORE) would be a structural failure. */
    assert(ehdr.e_type == ET_EXEC || ehdr.e_type == ET_DYN);

    /* Program header count must be non-zero on a loadable binary. */
    assert(ehdr.e_phnum > 0);

    assert(elf_end(e) == 0);
    close(fd);

    printf("elfutils GElf header probe OK (e_type=%u, e_phnum=%u)\n",
           (unsigned)ehdr.e_type, (unsigned)ehdr.e_phnum);
    return 0;
}
