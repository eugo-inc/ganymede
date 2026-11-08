/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for c_ares: strerror; calls ares_strerror().
 */

#include <ares.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    const char *ok_msg = ares_strerror(ARES_SUCCESS);
    assert(ok_msg != NULL);
    assert(strlen(ok_msg) > 0);

    const char *nomem_msg = ares_strerror(ARES_ENOMEM);
    assert(nomem_msg != NULL);
    assert(strlen(nomem_msg) > 0);

    const char *badfam_msg = ares_strerror(ARES_EBADFAMILY);
    assert(badfam_msg != NULL);
    assert(strlen(badfam_msg) > 0);

    /* Success vs failure must differ. */
    assert(strcmp(ok_msg, nomem_msg) != 0);

    /* Two distinct failure codes must differ — catches a partial stub. */
    assert(strcmp(nomem_msg, badfam_msg) != 0);

    printf("c_ares strerror test OK: ok=\"%s\" enomem=\"%s\" ebadfamily=\"%s\"\n",
           ok_msg, nomem_msg, badfam_msg);
    return 0;
}
