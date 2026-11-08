/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_event_stream: streaming decoder; calls aws_default_allocator(), aws_event_stream_library_init(), aws_byte_buf_from_c_str(), aws_event_stream_message_init().
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
    uint32_t observed_total_len;
    uint32_t observed_headers_len;
    uint32_t observed_message_crc;
    size_t payload_written;
    uint8_t payload_buf[64];
};

static void on_payload_segment(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_byte_buf *data,
    int8_t final_segment,
    void *user_data) {
    (void)decoder;
    (void)final_segment;
    struct probe_state *st = (struct probe_state *)user_data;
    assert(st->payload_written + data->len <= sizeof(st->payload_buf));
    memcpy(st->payload_buf + st->payload_written, data->buffer, data->len);
    st->payload_written += data->len;
}

static void on_prelude(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_event_stream_message_prelude *prelude,
    void *user_data) {
    (void)decoder;
    struct probe_state *st = (struct probe_state *)user_data;
    st->prelude_count++;
    st->observed_total_len = prelude->total_len;
    st->observed_headers_len = prelude->headers_len;
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
    struct probe_state *st = (struct probe_state *)user_data;
    st->complete_count++;
    st->observed_message_crc = message_crc;
}

static void on_error(
    struct aws_event_stream_streaming_decoder *decoder,
    struct aws_event_stream_message_prelude *prelude,
    int error_code,
    const char *message,
    void *user_data) {
    (void)decoder;
    (void)prelude;
    (void)error_code;
    (void)message;
    struct probe_state *st = (struct probe_state *)user_data;
    st->error_count++;
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);
    aws_event_stream_library_init(alloc);

    const char *payload_str = "demo-stream";
    struct aws_byte_buf payload = aws_byte_buf_from_c_str(payload_str);

    struct aws_event_stream_message msg;
    int rc = aws_event_stream_message_init(&msg, alloc, NULL, &payload);
    assert(rc == AWS_OP_SUCCESS);

    uint32_t total = aws_event_stream_message_total_length(&msg);
    uint32_t expected_crc = aws_event_stream_message_message_crc(&msg);
    assert(total >= 16);

    const uint8_t *wire = aws_event_stream_message_buffer(&msg);
    assert(wire != NULL);
    uint8_t *wire_copy = (uint8_t *)malloc(total);
    assert(wire_copy != NULL);
    memcpy(wire_copy, wire, total);
    aws_event_stream_message_clean_up(&msg);

    /* Wire the streaming decoder with all five callbacks. */
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

    /* Feed in two chunks to force the decoder's resumable-state path. Split
     * mid-message (just past the prelude) so the second pump finishes the
     * payload + trailing CRC. */
    size_t split = total / 2;
    assert(split > 0 && split < total);

    struct aws_byte_buf head = aws_byte_buf_from_array(wire_copy, split);
    rc = aws_event_stream_streaming_decoder_pump(&decoder, &head);
    assert(rc == AWS_OP_SUCCESS);

    struct aws_byte_buf tail = aws_byte_buf_from_array(wire_copy + split, total - split);
    rc = aws_event_stream_streaming_decoder_pump(&decoder, &tail);
    assert(rc == AWS_OP_SUCCESS);

    /* Exactly one prelude + one complete; no errors. */
    assert(st.prelude_count == 1);
    assert(st.complete_count == 1);
    assert(st.error_count == 0);

    /* Prelude metadata matches what the encoder produced. */
    assert(st.observed_total_len == total);
    assert(st.observed_headers_len == 0);

    assert(st.observed_message_crc == expected_crc);

    /* The entire payload was streamed back through on_payload_segment. */
    assert(st.payload_written == strlen(payload_str));
    assert(memcmp(st.payload_buf, payload_str, st.payload_written) == 0);

    aws_event_stream_streaming_decoder_clean_up(&decoder);
    free(wire_copy);
    aws_event_stream_library_clean_up();

    printf("aws_c_event_stream streaming-decoder test OK: total=%u crc=0x%08x payload=%zu\n",
           st.observed_total_len, st.observed_message_crc, st.payload_written);
    return 0;
}
