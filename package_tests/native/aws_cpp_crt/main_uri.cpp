/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_cpp_crt: uri; calls Aws::Crt::ByteCursorFromCString(), Aws::Crt::Io::EncodeQueryParameterValue().
 */

#include <aws/crt/Api.h>
#include <aws/crt/Types.h>
#include <aws/crt/io/Uri.h>
#include <cassert>
#include <cstring>
#include <iostream>

static bool cursor_equals(const aws_byte_cursor &c, const char *expected) {
    size_t n = std::strlen(expected);
    return c.len == n && std::memcmp(c.ptr, expected, n) == 0;
}

int main() {
    Aws::Crt::ApiHandle api;

    const char *raw = "https://example.com:8443/v1/items?key=val";
    aws_byte_cursor cursor = Aws::Crt::ByteCursorFromCString(raw);

    Aws::Crt::Io::Uri uri(cursor);
    assert(static_cast<bool>(uri));
    assert(uri.LastError() == 0);

    assert(cursor_equals(uri.GetScheme(), "https"));
    assert(cursor_equals(uri.GetHostName(), "example.com"));
    assert(uri.GetPort() == 8443);
    assert(cursor_equals(uri.GetPath(), "/v1/items"));
    assert(cursor_equals(uri.GetQueryString(), "key=val"));

    /* Query-parameter encoder: reserved chars must be percent-escaped. */
    aws_byte_cursor unsafe = Aws::Crt::ByteCursorFromCString("hello world=foo");
    Aws::Crt::String encoded = Aws::Crt::Io::EncodeQueryParameterValue(unsafe);
    assert(encoded.find("%20") != Aws::Crt::String::npos);  /* space */
    assert(encoded.find("%3D") != Aws::Crt::String::npos);  /* '=' */
    assert(encoded.find(' ') == Aws::Crt::String::npos);    /* no raw space */

    /* Negative test: a manifestly invalid URI must fail to parse. */
    aws_byte_cursor bad_cursor = Aws::Crt::ByteCursorFromCString("::::not-a-uri");
    Aws::Crt::Io::Uri bad(bad_cursor);
    assert(!static_cast<bool>(bad));
    assert(bad.LastError() != 0);

    std::cout << "aws_cpp_crt Uri test OK: " << raw << " -> port="
              << uri.GetPort() << ", encoded=" << encoded << std::endl;
    return 0;
}
