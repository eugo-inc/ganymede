/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_lambda_cpp: http response; compiles against <aws/http/response.h>.
 */

#include <aws/http/response.h>
#include <cassert>
#include <string>

// Exercises the header-only aws::http::response surface — case-insensitive
// header lookup, body accumulation, content-type round-trip, and the
// response_code enum that the lambda runtime uses for outcome failure
// signalling. Distinct from main.cpp (no invocation_response involvement at
// all — header-only path, catches breakage in aws/http/response.h).
int main(void) {
    aws::http::response r;

    r.set_response_code(aws::http::response_code::OK);
    assert(r.get_response_code() == aws::http::response_code::OK);
    // Sanity-check a couple of canonical enum members are reachable.
    assert(static_cast<int>(aws::http::response_code::OK) == 200);
    assert(static_cast<int>(aws::http::response_code::NOT_FOUND) == 404);
    assert(static_cast<int>(aws::http::response_code::REQUEST_NOT_MADE) == -1);

    r.set_content_type("application/json");
    assert(r.get_content_type() == "application/json");

    // add_header lower-cases the name; get_header lookups are by lower-case.
    r.add_header("X-Amzn-RequestId", "abc-123");
    assert(r.has_header("x-amzn-requestid"));
    auto hdr = r.get_header("x-amzn-requestid");
    assert(hdr.is_success());
    assert(hdr.get_result() == "abc-123");

    auto missing = r.get_header("x-not-there");
    assert(!missing.is_success());

    // Body accumulates from append_body chunks (the curl write-callback path).
    std::string chunk1 = "hello, ";
    std::string chunk2 = "lambda!";
    r.append_body(chunk1.data(), chunk1.size());
    r.append_body(chunk2.data(), chunk2.size());
    assert(r.get_body() == "hello, lambda!");

    return 0;
}
