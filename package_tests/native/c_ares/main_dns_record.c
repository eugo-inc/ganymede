/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for c_ares: dns record; calls ares_library_init(), ares_dns_record_create(), ares_dns_record_query_add(), ares_dns_record_query_cnt().
 */

#include <ares.h>
#include <ares_dns_record.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    assert(ares_library_init(ARES_LIB_INIT_ALL) == ARES_SUCCESS);

    ares_dns_record_t *rec = NULL;
    ares_status_t st = ares_dns_record_create(
        &rec, 0x1234, 0, ARES_OPCODE_QUERY, ARES_RCODE_NOERROR);
    assert(st == ARES_SUCCESS);
    assert(rec != NULL);

    /* --- Add a single AAAA question for "example.com" in IN class. */
    st = ares_dns_record_query_add(rec, "example.com",
                                   ARES_REC_TYPE_AAAA, ARES_CLASS_IN);
    assert(st == ARES_SUCCESS);
    assert(ares_dns_record_query_cnt(rec) == 1);

    const char *got_name = NULL;
    ares_dns_rec_type_t got_qtype = 0;
    ares_dns_class_t    got_qclass = 0;
    st = ares_dns_record_query_get(rec, 0, &got_name, &got_qtype, &got_qclass);
    assert(st == ARES_SUCCESS);
    assert(got_name != NULL);
    assert(strcmp(got_name, "example.com") == 0);
    assert(got_qtype  == ARES_REC_TYPE_AAAA);
    assert(got_qclass == ARES_CLASS_IN);

    assert(ares_dns_record_get_id(rec) == 0x1234);

    /* --- Enum<->string table sanity: AAAA must stringify to a non-empty
     *     string containing "AAAA", and parse back to the same enum value. */
    const char *type_str = ares_dns_rec_type_tostr(ARES_REC_TYPE_AAAA);
    assert(type_str != NULL);
    assert(strlen(type_str) > 0);
    assert(strstr(type_str, "AAAA") != NULL);

    ares_dns_rec_type_t parsed = 0;
    ares_bool_t ok = ares_dns_rec_type_fromstr(&parsed, type_str);
    assert(ok == ARES_TRUE);
    assert(parsed == ARES_REC_TYPE_AAAA);

    /* --- Class string sanity: ARES_CLASS_IN must produce non-empty text. */
    const char *class_str = ares_dns_class_tostr(ARES_CLASS_IN);
    assert(class_str != NULL);
    assert(strlen(class_str) > 0);

    /* Snapshot the inserted name before destroying the record, since
     * ares_dns_record_destroy invalidates the pointer ares_dns_record_query_get
     * returned. */
    char name_snapshot[256] = {0};
    strncpy(name_snapshot, got_name, sizeof(name_snapshot) - 1);

    ares_dns_record_destroy(rec);
    ares_library_cleanup();

    printf("c_ares dns_record test OK: name=%s qtype=%s qclass=%s\n",
           name_snapshot, type_str, class_str);
    return 0;
}
