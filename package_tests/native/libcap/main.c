/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libcap: core API; calls cap_from_name(), cap_to_name(), cap_free(), cap_from_text().
 */

#include <stdio.h>
#include <string.h>
#include <sys/capability.h>

int main(void) {
    if (LIBCAP_MAJOR != 2 || LIBCAP_MINOR != 78) {
        fprintf(stderr, "unexpected libcap header version %d.%d\n", LIBCAP_MAJOR, LIBCAP_MINOR);
        return 1;
    }
    cap_value_t value = 0;
    if (cap_from_name("CAP_Sys_Admin", &value) != 0 || value != CAP_SYS_ADMIN) {
        fprintf(stderr, "cap_from_name(\"CAP_Sys_Admin\") did not give CAP_SYS_ADMIN (%d)\n", (int)value);
        return 1;
    }
    char *name = cap_to_name(CAP_NET_RAW);
    if (name == NULL || strcmp(name, "cap_net_raw") != 0) {
        fprintf(stderr, "cap_to_name(CAP_NET_RAW) gave %s\n", name ? name : "(null)");
        return 1;
    }
    cap_free(name);
    cap_t set = cap_from_text("cap_net_raw,cap_sys_admin=ep");
    char *text = set ? cap_to_text(set, NULL) : NULL;
    if (text == NULL || strcmp(text, "cap_net_raw,cap_sys_admin=ep") != 0) {
        fprintf(stderr, "cap_from_text/cap_to_text round trip gave %s\n", text ? text : "(null)");
        return 1;
    }
    cap_free(text);
    cap_free(set);
    cap_t mine = cap_get_proc();
    if (mine == NULL) {
        perror("cap_get_proc");
        return 1;
    }
    char *mine_text = cap_to_text(mine, NULL);
    printf("libcap %d.%d; cap_max_bits %d; this process: %s\n", LIBCAP_MAJOR, LIBCAP_MINOR, (int)cap_max_bits(),
           mine_text ? mine_text : "(unreadable)");
    cap_free(mine_text);
    cap_free(mine);
    return 0;
}
