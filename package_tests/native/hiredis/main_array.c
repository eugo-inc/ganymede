/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hiredis: array; calls redisReaderCreate(), redisReaderFeed(), redisReaderGetReply(), freeReplyObject().
 */

#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    static const char wire[] =
        "*3\r\n"
        "$3\r\nfoo\r\n"
        "$3\r\nbar\r\n"
        "$3\r\nbaz\r\n";

    redisReader *reader = redisReaderCreate();
    assert(reader != NULL);

    assert(redisReaderFeed(reader, wire, sizeof(wire) - 1) == REDIS_OK);

    void *reply = NULL;
    assert(redisReaderGetReply(reader, &reply) == REDIS_OK);
    assert(reply != NULL);

    redisReply *r = (redisReply *)reply;
    assert(r->type == REDIS_REPLY_ARRAY);
    assert(r->elements == 3);
    assert(r->element != NULL);

    static const char *const expected[3] = {"foo", "bar", "baz"};
    for (size_t i = 0; i < 3; ++i) {
        redisReply *child = r->element[i];
        assert(child != NULL);
        assert(child->type == REDIS_REPLY_STRING);
        assert(child->len == 3);
        assert(strcmp(child->str, expected[i]) == 0);
    }

    freeReplyObject(reply);
    redisReaderFree(reader);

    printf("hiredis array test OK\n");
    return 0;
}
