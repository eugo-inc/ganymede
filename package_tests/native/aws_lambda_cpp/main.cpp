/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_lambda_cpp: core API; calls aws::lambda_runtime::invocation_response::success().
 */

#include <aws/lambda-runtime/runtime.h>
#include <cassert>
#include <iostream>

int main(void) {
    auto resp = aws::lambda_runtime::invocation_response::success("ok", "application/json");
    assert(!resp.get_payload().empty());
    std::cout << "aws_lambda_cpp test OK" << std::endl;
    return 0;
}
