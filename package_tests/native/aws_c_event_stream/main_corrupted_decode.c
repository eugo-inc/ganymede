/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_event_stream: corrupted decode; calls aws_default_allocator(), aws_event_stream_library_init(), aws_event_stream_message_init(), aws_event_stream_message_total_length().
 */

#include <aws/event-stream/event_stream.h>
#include <aws/common/array_list.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct probe_state {
    int prelude_count;
    int complete_count;
    int error_count;
    int observed_error_code;
};

static void on_payload_segment(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_byte_buf *data,
    int8_t final_segment,
    void *user_data) {
    (void)decoder;
    (void)data;
    (void)final_segment;
    (void)user_data;
}

static void on_prelude(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_event_stream_message_prelude *prelude,
    void *user_data) {
    (void)decoder;
    (void)prelude;
    struct probe_state *st = (struct probe_state *)user_data;
    st->prelude_count++;
}

static void on_header(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_event_stream_message_prelude *prelude,
    struct aws_event_stream_header_value_pair *header,
    void *user_data) {
    (void)decoder;
    (void)prelude;
    (void)header;
    (void)user_data;
}

static void on_complete(
    struct aws_event_stream_streaming_decoder *decoder,
    uint32_t message_crc,
    void *user_data) {
    (void)decoder;
    (void)message_crc;
    struct probe_state *st = (struct probe_state *)user_data;
    st->complete_count++;
}

static void on_error(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_event_stream_message_prelude *prelude,
    int error_code,
    const char *message,
    void *user_data) {
    (void)decoder;
    (void)prelude;
    (void)message;
    struct probe_state *st = (struct probe_state *)user_data;
    st->error_count++;
    st->observed_error_code = error_code;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_event_stream_library_init(alloc);

    struct aws_event_stream_message msg;
    int rc = aws_event_stream_message_init(&msg, alloc, NULL, NULL);
    assert(rc == AWS_OP_SUCCESS);
    uint32_t total = aws_event_stream_message_total_length(&msg);
    assert(total == 16);

    const uint8_t *wire = aws_event_stream_message_buffer(&msg);
    assert(wire != NULL);
    uint8_t *wire_copy = (uint8_t *)malloc(total);
    assert(wire_copy != NULL);
    memcpy(wire_copy, wire, total);
    aws_event_stream_message_clean_up(&msg);

    /* Corrupt the prelude CRC (bytes [8..11] of the 12-byte prelude). Flip
     * a single bit so prelude-CRC verification will fail. */
    wire_copy[8] ^= 0xFF;

    struct probe_state st;
    memset(&st, 0, sizeof(st));

    struct aws_event_stream_streaming_decoder_options opts = {
        .on_payload_segment = on_payload_segment,
        .on_prelude = on_prelude,
        .on_header = on_header,
        .on_complete = on_complete,
        .on_error = on_error,
        .user_data = &st,
    };

    struct aws_event_stream_streaming_decoder decoder;
    aws_event_stream_streaming_decoder_init_from_options(&decoder, alloc, &opts);

    struct aws_byte_buf bad = aws_byte_buf_from_array(wire_copy, total);
    /* The pump call itself is allowed to return AWS_OP_ERR — the contract
     * is just that on_error fires. Don't assert rc here. */
    (void)aws_event_stream_streaming_decoder_pump(&decoder, &bad);

    /* The decoder MUST have surfaced the corruption via on_error, and MUST
     * NOT have signaled a successful complete. */
    assert(st.error_count == 1);
    assert(st.complete_count == 0);
    assert(st.observed_error_code == AWS_ERROR_EVENT_STREAM_PRELUDE_CHECKSUM_FAILURE);
    /* Prelude callback is only invoked once the prelude has passed the CRC
     * check; corrupted prelude must bypass it. */
    assert(st.prelude_count == 0);

    aws_event_stream_streaming_decoder_clean_up(&decoder);
    free(wire_copy);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream corrupted-decode test OK: errors=%d code=%d\n",
           st.error_count, st.observed_error_code);
    return 0;
}
