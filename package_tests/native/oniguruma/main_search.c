/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for oniguruma: search; calls onig_initialize(), onig_new(), onig_error_code_to_str(), onig_region_new().
 */

#include <oniguruma.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    OnigEncoding encodings[] = {ONIG_ENCODING_UTF8};
    if (onig_initialize(encodings, 1) != ONIG_NORMAL) {
        fprintf(stderr, "FAIL: onig_initialize\n");
        return 2;
    }

    regex_t *reg = NULL;
    OnigErrorInfo einfo;
    const UChar *pattern = (const UChar *)"a(\\d+)b";
    int r = onig_new(&reg, pattern, pattern + strlen((const char *)pattern), ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8,
                     ONIG_SYNTAX_DEFAULT, &einfo);
    if (r != ONIG_NORMAL) {
        UChar message[ONIG_MAX_ERROR_MESSAGE_LEN];
        onig_error_code_to_str(message, r, &einfo);
        fprintf(stderr, "FAIL: onig_new: %s\n", (const char *)message);
        return 3;
    }

    OnigRegion *region = onig_region_new();
    if (region == NULL) {
        fprintf(stderr, "FAIL: onig_region_new returned NULL\n");
        onig_free(reg);
        return 4;
    }

    int status = 0;

    const UChar *hit = (const UChar *)"xxa123byy";
    const UChar *hit_end = hit + strlen((const char *)hit);
    r = onig_search(reg, hit, hit_end, hit, hit_end, region, ONIG_OPTION_NONE);
    if (r != 2 || region->num_regs != 2 || region->beg[1] != 3 || region->end[1] != 6) {
        fprintf(stderr, "FAIL: search of `%s` gave %d (regs=%d, group1=[%d,%d)), expected 2 with group1=[3,6)\n",
                (const char *)hit, r, region->num_regs, region->num_regs > 1 ? region->beg[1] : -1,
                region->num_regs > 1 ? region->end[1] : -1);
        status = 5;
    }

    onig_region_clear(region);
    const UChar *miss = (const UChar *)"xxab123yy";
    const UChar *miss_end = miss + strlen((const char *)miss);
    r = onig_search(reg, miss, miss_end, miss, miss_end, region, ONIG_OPTION_NONE);
    if (status == 0 && r != ONIG_MISMATCH) {
        fprintf(stderr, "FAIL: search of `%s` gave %d, expected ONIG_MISMATCH (%d)\n", (const char *)miss, r,
                ONIG_MISMATCH);
        status = 6;
    }

    onig_region_free(region, 1);
    onig_free(reg);
    onig_end();

    if (status == 0) {
        printf("oniguruma OK: a(\\d+)b matched xxa123byy at 2, group1=[3,6); xxab123yy did not match\n");
    }
    return status;
}
