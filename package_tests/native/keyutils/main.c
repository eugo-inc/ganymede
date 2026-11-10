/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for keyutils: core API; calls keyctl_get_keyring_ID().
 */

#include <errno.h>
#include <keyutils.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    if (strstr(keyutils_version_string, "keyutils-1.6.3") == NULL) {
        fprintf(stderr, "unexpected keyutils_version_string: %s\n", keyutils_version_string);
        return 1;
    }
    key_serial_t session = keyctl_get_keyring_ID(KEY_SPEC_SESSION_KEYRING, 0);
    if (session < 0 && errno == ENOSYS) {
        fprintf(stderr, "keyctl_get_keyring_ID: the kernel does not implement the key system calls (ENOSYS)\n");
        return 1;
    }
    printf("%s; session keyring %s\n", keyutils_version_string, session < 0 ? strerror(errno) : "reachable");
    return 0;
}
