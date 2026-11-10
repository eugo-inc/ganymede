/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for pam: core API; calls pam_start(), pam_get_item(), pam_strerror(), pam_end().
 */

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    if (__LINUX_PAM__ != 1 || __LINUX_PAM_MINOR__ != 7) {
        fprintf(stderr, "unexpected Linux-PAM header version %d.%d\n", __LINUX_PAM__, __LINUX_PAM_MINOR__);
        return 1;
    }
    struct pam_conv conversation = {misc_conv, NULL};
    pam_handle_t *handle = NULL;
    int rc = pam_start("demo-native-test", "nobody", &conversation, &handle);
    if (rc != PAM_SUCCESS || handle == NULL) {
        fprintf(stderr, "pam_start failed: %d\n", rc);
        return 1;
    }
    const void *service = NULL;
    if (pam_get_item(handle, PAM_SERVICE, &service) != PAM_SUCCESS || service == NULL ||
        strcmp((const char *)service, "demo-native-test") != 0) {
        fprintf(stderr, "pam_get_item(PAM_SERVICE) did not return the service we started\n");
        return 1;
    }
    const char *message = pam_strerror(handle, PAM_AUTH_ERR);
    if (message == NULL || strcmp(message, "Authentication failure") != 0) {
        fprintf(stderr, "pam_strerror(PAM_AUTH_ERR) gave %s\n", message ? message : "(null)");
        return 1;
    }
    if (pam_end(handle, PAM_SUCCESS) != PAM_SUCCESS) {
        fprintf(stderr, "pam_end failed\n");
        return 1;
    }
    printf("pam OK: Linux-PAM %d.%d, a conversation started and ended\n", __LINUX_PAM__, __LINUX_PAM_MINOR__);
    return 0;
}
