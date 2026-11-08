/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for pcre2: core API; calls pcre2_compile(), pcre2_match_data_create_from_pattern(), pcre2_match(), pcre2_get_ovector_pointer().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    PCRE2_SPTR pattern = (PCRE2_SPTR)"hello\\s+(\\w+)";
    PCRE2_SPTR subject = (PCRE2_SPTR)"hello world";

    int errornumber = 0;
    PCRE2_SIZE erroroffset = 0;
    pcre2_code *re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0,
                                   &errornumber, &erroroffset, NULL);
    assert(re != NULL);

    pcre2_match_data *md = pcre2_match_data_create_from_pattern(re, NULL);
    assert(md != NULL);

    int rc = pcre2_match(re, subject, strlen((const char *)subject),
                         0, 0, md, NULL);
    assert(rc == 2); /* whole match + one capturing group */

    PCRE2_SIZE *ov = pcre2_get_ovector_pointer(md);
    PCRE2_SIZE group_len = ov[3] - ov[2];
    assert(group_len == 5);
    assert(memcmp(subject + ov[2], "world", 5) == 0);

    pcre2_match_data_free(md);
    pcre2_code_free(re);

    /* Version string must contain "10." (PCRE2 has been on the 10.x line since release). */
    char version[64] = {0};
    int vlen = pcre2_config(PCRE2_CONFIG_VERSION, version);
    assert(vlen > 0);
    assert(strstr(version, "10.") != NULL);

    printf("pcre2 test OK: %s\n", version);
    return 0;
}
