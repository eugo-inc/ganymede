/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libabigail: version; compiles against <libabigail/abg-version.h>.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-version.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* The substituted macros must be real digit strings, not unexpanded "@FOO@" placeholders. */
static int is_unexpanded_placeholder(const char *s) {
    return s != NULL && s[0] == '@';
}

static int is_all_digits(const char *s) {
    if (s == NULL || *s == '\0') return 0;
    for (; *s != '\0'; ++s) {
        if (*s < '0' || *s > '9') return 0;
    }
    return 1;
}

int main(void) {
    assert(strlen(ABIGAIL_VERSION_MAJOR) > 0);
    assert(strlen(ABIGAIL_VERSION_MINOR) > 0);
    assert(strlen(ABIGAIL_ABIXML_VERSION_MAJOR) > 0);
    assert(strlen(ABIGAIL_ABIXML_VERSION_MINOR) > 0);

    /* autoconf must have substituted them — no leftover "@VERSION_MAJOR@" etc. */
    assert(!is_unexpanded_placeholder(ABIGAIL_VERSION_MAJOR));
    assert(!is_unexpanded_placeholder(ABIGAIL_VERSION_MINOR));
    assert(!is_unexpanded_placeholder(ABIGAIL_ABIXML_VERSION_MAJOR));
    assert(!is_unexpanded_placeholder(ABIGAIL_ABIXML_VERSION_MINOR));

    /* Major/minor components of both the library version AND the abixml
       schema version must be plain decimal digit strings. */
    assert(is_all_digits(ABIGAIL_VERSION_MAJOR));
    assert(is_all_digits(ABIGAIL_VERSION_MINOR));
    assert(is_all_digits(ABIGAIL_ABIXML_VERSION_MAJOR));
    assert(is_all_digits(ABIGAIL_ABIXML_VERSION_MINOR));

    printf("libabigail version header OK: lib=%s.%s abixml=%s.%s\n",
           ABIGAIL_VERSION_MAJOR, ABIGAIL_VERSION_MINOR,
           ABIGAIL_ABIXML_VERSION_MAJOR, ABIGAIL_ABIXML_VERSION_MINOR);
    return 0;
}
