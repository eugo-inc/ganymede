/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for civetweb: core API; calls mg_init_library(), mg_version(), mg_exit_library().
 */

#include <civetweb.h>
#include <stdio.h>

int main(void) {
    /* civetweb's mg_init_library returns a feature mask. Don't assert about its value;
       just verify the call doesn't crash + report the library version. */
    unsigned features = mg_init_library(MG_FEATURES_DEFAULT);
    (void)features;
    printf("civetweb test OK: %s\n", mg_version());
    mg_exit_library();
    return 0;
}
