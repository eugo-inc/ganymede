/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libogg: roundtrip; calls ogg_stream_init(), ogg_sync_init(), ogg_stream_packetin(), ogg_stream_flush().
 */

#include <ogg/ogg.h>

#include <stdio.h>
#include <string.h>

#define SERIAL 0x45554730 /* "EUG0" */

int main(void) {
    unsigned char payload[300];
    for (size_t i = 0; i < sizeof payload; ++i) {
        payload[i] = (unsigned char)(i * 7u);
    }

    ogg_stream_state writer;
    ogg_stream_state reader;
    ogg_sync_state sync;
    if (ogg_stream_init(&writer, SERIAL) != 0 || ogg_stream_init(&reader, SERIAL) != 0 || ogg_sync_init(&sync) != 0) {
        fprintf(stderr, "ogg init failed\n");
        return 1;
    }

    ogg_packet packet;
    memset(&packet, 0, sizeof packet);
    packet.packet = payload;
    packet.bytes = (long)sizeof payload;
    packet.b_o_s = 1;
    packet.e_o_s = 1;
    if (ogg_stream_packetin(&writer, &packet) != 0) {
        fprintf(stderr, "ogg_stream_packetin failed\n");
        return 1;
    }

    ogg_page page;
    if (ogg_stream_flush(&writer, &page) == 0) {
        fprintf(stderr, "ogg_stream_flush produced no page\n");
        return 1;
    }

    const long page_bytes = page.header_len + page.body_len;
    char *buffer = ogg_sync_buffer(&sync, page_bytes);
    if (buffer == NULL) {
        fprintf(stderr, "ogg_sync_buffer returned NULL\n");
        return 1;
    }
    memcpy(buffer, page.header, (size_t)page.header_len);
    memcpy(buffer + page.header_len, page.body, (size_t)page.body_len);
    ogg_sync_wrote(&sync, page_bytes);

    ogg_page read_page;
    ogg_packet read_packet;
    if (ogg_sync_pageout(&sync, &read_page) != 1 || ogg_stream_pagein(&reader, &read_page) != 0 ||
        ogg_stream_packetout(&reader, &read_packet) != 1) {
        fprintf(stderr, "reading the page back failed\n");
        return 1;
    }
    if (read_packet.bytes != (long)sizeof payload || memcmp(read_packet.packet, payload, sizeof payload) != 0) {
        fprintf(stderr, "the packet read back differs from the one written (%ld bytes)\n", read_packet.bytes);
        return 1;
    }

    ogg_stream_clear(&writer);
    ogg_stream_clear(&reader);
    ogg_sync_clear(&sync);

    printf("libogg: %zu-byte packet round-tripped through a %ld-byte page\n", sizeof payload, page_bytes);
    return 0;
}
