/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for ucx: core API; calls ucp_get_version(), ucp_config_read(), ucs_status_string(), ucp_config_release().
 */

#include <ucp/api/ucp.h>

#include <stdio.h>

int main(void) {
    unsigned major = 0;
    unsigned minor = 0;
    unsigned release = 0;
    ucp_get_version(&major, &minor, &release);
    if (major != UCP_API_MAJOR || minor != UCP_API_MINOR) {
        fprintf(stderr, "ucx: library %u.%u.%u but ucp headers %u.%u\n", major, minor, release, (unsigned) UCP_API_MAJOR,
                (unsigned) UCP_API_MINOR);
        return 1;
    }

    ucp_config_t *config = NULL;
    const ucs_status_t status = ucp_config_read(NULL, NULL, &config);
    if (status != UCS_OK || config == NULL) {
        fprintf(stderr, "ucx: ucp_config_read failed: %s\n", ucs_status_string(status));
        return 1;
    }
    ucp_config_release(config);

    printf("ucx: %s (headers %u.%u), configuration parser OK\n", ucp_get_version_string(), (unsigned) UCP_API_MAJOR,
           (unsigned) UCP_API_MINOR);
    return 0;
}
