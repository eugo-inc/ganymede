/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for llhttp: request; calls llhttp_settings_init(), llhttp_init(), llhttp_execute(), llhttp_errno_name().
 */

#include <llhttp.h>

#include <stdio.h>

static int completed_messages = 0;

static int on_message_complete(llhttp_t *parser) {
    (void)parser;
    ++completed_messages;
    return 0;
}

int main(void) {
    llhttp_settings_t settings;
    llhttp_settings_init(&settings);
    settings.on_message_complete = on_message_complete;

    llhttp_t parser;
    llhttp_init(&parser, HTTP_REQUEST, &settings);

    static const char request[] = "POST /v1/completions HTTP/1.1\r\n"
                                  "Host: localhost\r\n"
                                  "Content-Length: 2\r\n"
                                  "\r\n"
                                  "{}";
    const llhttp_errno_t error = llhttp_execute(&parser, request, sizeof request - 1);
    if (error != HPE_OK) {
        fprintf(stderr, "llhttp_execute: %s (%s)\n", llhttp_errno_name(error), llhttp_get_error_reason(&parser));
        return 1;
    }
    if (completed_messages != 1 || llhttp_get_method(&parser) != HTTP_POST) {
        fprintf(stderr, "expected one complete POST, got %d message(s), method %u\n", completed_messages,
                (unsigned)llhttp_get_method(&parser));
        return 1;
    }

    printf("llhttp %d.%d.%d: parsed a POST with a body\n", LLHTTP_VERSION_MAJOR, LLHTTP_VERSION_MINOR,
           LLHTTP_VERSION_PATCH);
    return 0;
}
