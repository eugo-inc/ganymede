/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libexpat: symbol versions; calls XML_ExpatVersion(), dlinfo(), dlvsym().
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <elf.h>
#include <expat.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tag_probe {
    const char *tag;
    const char *function;
};

static const struct tag_probe k_expected[] = {
    {"LIBEXPAT_1.0.0", "XML_ParserCreate"},
    {"LIBEXPAT_1.1.0", "XML_ParserCreateNS"},
    {"LIBEXPAT_1.95.0", "XML_ExpatVersion"},
    {"LIBEXPAT_1.95.3", "XML_ExpatVersionInfo"},
    {"LIBEXPAT_1.95.4", "XML_SetSkippedEntityHandler"},
    {"LIBEXPAT_1.95.5", "XML_GetFeatureList"},
    {"LIBEXPAT_1.95.6", "XML_MemMalloc"},
    {"LIBEXPAT_1.95.8", "XML_StopParser"},
    {"LIBEXPAT_2.1.0", "XML_SetHashSalt"},
    {"LIBEXPAT_2.4.0", "XML_SetBillionLaughsAttackProtectionMaximumAmplification"},
    {"LIBEXPAT_2.6.0", "XML_SetReparseDeferralEnabled"},
    {"LIBEXPAT_2.7.2", "XML_SetAllocTrackerMaximumAmplification"},
    {"LIBEXPAT_2.8.0", "XML_SetHashSalt16Bytes"},
};
#define EXPECTED_COUNT (sizeof(k_expected) / sizeof(k_expected[0]))

static int read_version_definitions(const char *path, char names[][64], int cap) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        fprintf(stderr, "cannot open %s\n", path);
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long size = ftell(f);
    rewind(f);
    unsigned char *data = (size > 0) ? malloc((size_t)size) : NULL;
    if (data == NULL || fread(data, 1, (size_t)size, f) != (size_t)size) {
        fprintf(stderr, "cannot read %s\n", path);
        free(data);
        fclose(f);
        return -1;
    }
    fclose(f);

    const Elf64_Ehdr *eh = (const Elf64_Ehdr *)data;
    if ((size_t)size < sizeof(*eh) || memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0 || eh->e_ident[EI_CLASS] != ELFCLASS64 ||
        eh->e_shoff == 0 || eh->e_shoff + (Elf64_Off)eh->e_shnum * sizeof(Elf64_Shdr) > (Elf64_Off)size) {
        fprintf(stderr, "%s is not a readable 64-bit ELF with section headers\n", path);
        free(data);
        return -1;
    }
    const Elf64_Shdr *sh = (const Elf64_Shdr *)(data + eh->e_shoff);
    int found = 0;
    for (int i = 0; i < eh->e_shnum; i++) {
        if (sh[i].sh_type != SHT_GNU_verdef || sh[i].sh_link >= eh->e_shnum) {
            continue;
        }
        const Elf64_Shdr *strtab = &sh[sh[i].sh_link];
        Elf64_Off offset = sh[i].sh_offset;
        for (Elf64_Word n = 0; n < sh[i].sh_info && offset + sizeof(Elf64_Verdef) <= (Elf64_Off)size; n++) {
            const Elf64_Verdef *vd = (const Elf64_Verdef *)(data + offset);
            Elf64_Off aux = offset + vd->vd_aux;
            if (aux + sizeof(Elf64_Verdaux) <= (Elf64_Off)size && found < cap) {
                const Elf64_Verdaux *va = (const Elf64_Verdaux *)(data + aux);
                Elf64_Off name = strtab->sh_offset + va->vda_name;
                if (name < (Elf64_Off)size) {
                    snprintf(names[found], 64, "%s", (const char *)(data + name));
                    found++;
                }
            }
            if (vd->vd_next == 0) {
                break;
            }
            offset += vd->vd_next;
        }
    }
    free(data);
    return found;
}

int main(void) {
    /* Calling the library keeps it a real dependency of this program, so the loader maps it at start-up. */
    const XML_LChar *version = XML_ExpatVersion();

    void *handle = dlopen("libexpat.so.1", RTLD_NOW | RTLD_NOLOAD);
    struct link_map *map = NULL;
    if (handle == NULL || dlinfo(handle, RTLD_DI_LINKMAP, &map) != 0 || map == NULL || map->l_name[0] == '\0') {
        fprintf(stderr, "libexpat.so.1 is not mapped into this process: %s\n", dlerror());
        return 2;
    }
    const char *path = map->l_name;

    char names[64][64];
    int defined = read_version_definitions(path, names, 64);
    if (defined < 0) {
        return 2;
    }

    int failures = 0;
    char missing[1024] = "";
    int missing_count = 0;
    for (size_t i = 0; i < EXPECTED_COUNT; i++) {
        int present = 0;
        for (int j = 0; j < defined; j++) {
            present |= (strcmp(names[j], k_expected[i].tag) == 0);
        }
        if (!present) {
            missing_count++;
            strncat(missing, " ", sizeof(missing) - strlen(missing) - 1);
            strncat(missing, k_expected[i].tag, sizeof(missing) - strlen(missing) - 1);
        }
    }
    if (missing_count > 0) {
        fprintf(stderr,
                "UNVERSIONED libexpat: %s defines %d version tags, %d of the %zu expected LIBEXPAT_* tags missing "
                "(built without -DEXPAT_SYMBOL_VERSIONING=ON?):%s\n",
                path, defined, missing_count, EXPECTED_COUNT, missing);
        failures++;
    }

    for (size_t i = 0; i < EXPECTED_COUNT; i++) {
        if (dlvsym(handle, k_expected[i].function, k_expected[i].tag) == NULL) {
            fprintf(stderr, "dlvsym(%s, %s) found nothing: that tag does not hold that function\n",
                    k_expected[i].function, k_expected[i].tag);
            failures++;
        }
    }

    /* The discriminating check: the wrong tag must be refused. XML_ParserCreate lives in LIBEXPAT_1.0.0. */
    if (dlvsym(handle, "XML_ParserCreate", "LIBEXPAT_2.8.0") != NULL) {
        fprintf(stderr,
                "dlvsym(XML_ParserCreate, LIBEXPAT_2.8.0) resolved although that function belongs to LIBEXPAT_1.0.0: "
                "%s has no symbol versions\n",
                path);
        failures++;
    }

    if (failures > 0) {
        return 1;
    }
    printf("libexpat symbol versions OK: %s (%s) defines %d version entries, all %zu expected LIBEXPAT_* tags present\n",
           path, version, defined, EXPECTED_COUNT);
    return 0;
}
