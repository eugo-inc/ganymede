/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_auth: static provider; calls aws_credentials_get_access_key_id(), aws_credentials_get_secret_access_key(), aws_byte_buf_init_copy_from_cursor(), aws_default_allocator().
 */

#include <aws/auth/auth.h>
#include <aws/auth/credentials.h>
#include <aws/common/allocator.h>
#include <aws/common/byte_buf.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct callback_state {
    int fired;
    int error_code;
    struct aws_byte_buf access_key_copy;
    struct aws_byte_buf secret_key_copy;
};

static void on_creds(struct aws_credentials *credentials, int error_code, void *user_data) {
    struct callback_state *state = user_data;
    state->fired = 1;
    state->error_code = error_code;

    if (credentials != NULL) {
        struct aws_byte_cursor ak = aws_credentials_get_access_key_id(credentials);
        struct aws_byte_cursor sk = aws_credentials_get_secret_access_key(credentials);
        aws_byte_buf_init_copy_from_cursor(&state->access_key_copy, aws_default_allocator(), ak);
        aws_byte_buf_init_copy_from_cursor(&state->secret_key_copy, aws_default_allocator(), sk);
    }
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_auth_library_init(alloc);

    struct aws_credentials_provider_static_options opts;
    AWS_ZERO_STRUCT(opts);
    opts.access_key_id = aws_byte_cursor_from_c_str("AKIAIOSFODNN7EXAMPLE");
    opts.secret_access_key = aws_byte_cursor_from_c_str("wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY");
    opts.session_token = aws_byte_cursor_from_c_str("");

    struct aws_credentials_provider *provider = aws_credentials_provider_new_static(alloc, &opts);
    assert(provider != NULL);

    struct callback_state state;
    AWS_ZERO_STRUCT(state);
    int rc = aws_credentials_provider_get_credentials(provider, on_creds, &state);
    assert(rc == AWS_OP_SUCCESS);

    /* Static provider fires the callback synchronously — no event loop needed. */
    assert(state.fired == 1);
    assert(state.error_code == 0);

    assert(state.access_key_copy.len == opts.access_key_id.len);
    assert(memcmp(state.access_key_copy.buffer, opts.access_key_id.ptr, opts.access_key_id.len) == 0);

    assert(state.secret_key_copy.len == opts.secret_access_key.len);
    assert(memcmp(state.secret_key_copy.buffer, opts.secret_access_key.ptr, opts.secret_access_key.len) == 0);

    aws_byte_buf_clean_up(&state.access_key_copy);
    aws_byte_buf_clean_up(&state.secret_key_copy);

    aws_credentials_provider_release(provider);
    aws_auth_library_clean_up();

    printf("aws_c_auth static_provider test OK\n");
    return 0;
}
