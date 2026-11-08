/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_cpp_crt: json; calls Aws::Crt::String().
 */

#include <aws/crt/Api.h>
#include <aws/crt/JsonObject.h>
#include <cassert>
#include <iostream>

int main() {
    Aws::Crt::ApiHandle api;

    Aws::Crt::JsonObject obj;
    obj.WithString("name", Aws::Crt::String("demo"))
        .WithBool("ok", true)
        .WithInteger("answer", 42);

    Aws::Crt::String serialized = obj.View().WriteCompact();
    assert(!serialized.empty());
    /* Compact form must include all three keys and a closing brace. */
    assert(serialized.find("\"name\"") != Aws::Crt::String::npos);
    assert(serialized.find("\"demo\"") != Aws::Crt::String::npos);
    assert(serialized.find("\"ok\"") != Aws::Crt::String::npos);
    assert(serialized.find("\"answer\"") != Aws::Crt::String::npos);
    assert(serialized.find("42") != Aws::Crt::String::npos);

    Aws::Crt::JsonObject parsed(serialized);
    assert(parsed.WasParseSuccessful());

    Aws::Crt::JsonView view = parsed.View();
    assert(view.GetString("name") == Aws::Crt::String("demo"));
    assert(view.GetBool("ok") == true);
    assert(view.GetInteger("answer") == 42);

    /* Negative test: deliberately malformed JSON must fail to parse. */
    Aws::Crt::JsonObject bad(Aws::Crt::String("{not valid json"));
    assert(!bad.WasParseSuccessful());

    std::cout << "aws_cpp_crt JsonObject test OK: " << serialized << std::endl;
    return 0;
}
