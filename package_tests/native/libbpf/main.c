/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libbpf: core API; calls libbpf_major_version(), libbpf_minor_version(), btf__new_empty(), btf__add_int().
 */

#include <bpf/btf.h>
#include <bpf/libbpf.h>
#include <bpf/libbpf_version.h>

#include <stdio.h>
#include <string.h>

int main(void) {
    if (libbpf_major_version() != LIBBPF_MAJOR_VERSION || libbpf_minor_version() != LIBBPF_MINOR_VERSION) {
        fprintf(stderr, "libbpf: the loaded library is %u.%u but the headers are %d.%d\n", libbpf_major_version(),
                libbpf_minor_version(), LIBBPF_MAJOR_VERSION, LIBBPF_MINOR_VERSION);
        return 1;
    }

    struct btf *btf = btf__new_empty();
    if (btf == NULL) {
        fprintf(stderr, "libbpf: btf__new_empty failed\n");
        return 1;
    }

    int int_id = btf__add_int(btf, "int", 4, BTF_INT_SIGNED);
    int struct_id = btf__add_struct(btf, "demo_probe", 4);
    if (int_id <= 0 || struct_id <= 0 || btf__add_field(btf, "value", int_id, 0, 0) != 0) {
        fprintf(stderr, "libbpf: building the BTF failed (int %d, struct %d)\n", int_id, struct_id);
        btf__free(btf);
        return 1;
    }

    int found_id = btf__find_by_name_kind(btf, "demo_probe", BTF_KIND_STRUCT);
    const struct btf_type *found = found_id > 0 ? btf__type_by_id(btf, found_id) : NULL;
    if (found_id != struct_id || found == NULL || btf_vlen(found) != 1) {
        fprintf(stderr, "libbpf: the struct was not found back (id %d, expected %d)\n", found_id, struct_id);
        btf__free(btf);
        return 1;
    }
    const struct btf_member *member = btf_members(found);
    const char *member_name = btf__name_by_offset(btf, member->name_off);
    if (member_name == NULL || strcmp(member_name, "value") != 0 || member->type != (unsigned int) int_id) {
        fprintf(stderr, "libbpf: the struct's member does not read back as `value` of type int\n");
        btf__free(btf);
        return 1;
    }

    printf("libbpf %u.%u: a BTF of %u types round-tripped\n", libbpf_major_version(), libbpf_minor_version(),
           btf__type_cnt(btf));
    btf__free(btf);
    return 0;
}
