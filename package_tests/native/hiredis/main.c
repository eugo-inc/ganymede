/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for hiredis: core API; calls redisReaderCreate(), redisReaderFeed(), redisReaderGetReply(), freeReplyObject().
 */

#include <hiredis/hiredis.h>
#include <hiredis/read.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(void) {
    /* RESP simple-string reply: "+PONG\r\n" — what a real Redis server returns for PING. */
    static const char wire[] = "+PONG\r\n";

    redisReader *reader = redisReaderCreate();
    assert(reader != NULL);

    assert(redisReaderFeed(reader, wire, sizeof(wire) - 1) == REDIS_OK);

    void *reply = NULL;
    assert(redisReaderGetReply(reader, &reply) == REDIS_OK);
    assert(reply != NULL);

    redisReply *r = (redisReply *)reply;
    assert(r->type == REDIS_REPLY_STATUS);
    assert(r->len == 4);
    assert(strcmp(r->str, "PONG") == 0);

    freeReplyObject(reply);
    redisReaderFree(reader);

    printf("hiredis test OK\n");
    return 0;
}
