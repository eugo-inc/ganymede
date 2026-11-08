/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for aws_lambda_cpp: outcome; compiles against <aws/lambda-runtime/outcome.h>.
 */

#include <aws/lambda-runtime/outcome.h>
#include <cassert>
#include <string>
#include <utility>

// Exercises the header-only aws::lambda_runtime::outcome<TResult, TFailure>
// tagged-union template — the result/failure discriminator the runtime uses
// for next_outcome / post_outcome. Distinct API surface from main.cpp
// (templated class with placement-new union storage; catches breakage in
// outcome.h's copy/move and destructor logic).
int main(void) {
    using outcome_t = aws::lambda_runtime::outcome<std::string, int>;

    // Success branch.
    outcome_t ok(std::string("good-payload"));
    assert(ok.is_success());
    assert(ok.get_result() == "good-payload");

    // Failure branch — same type, different active union member.
    outcome_t err(42);
    assert(!err.is_success());
    assert(err.get_failure() == 42);

    // Copy ctor preserves success-ness AND the held value (placement-new
    // path inside the union — easy to break with a bad refactor).
    outcome_t ok_copy(ok);
    assert(ok_copy.is_success());
    assert(ok_copy.get_result() == "good-payload");

    outcome_t err_copy(err);
    assert(!err_copy.is_success());
    assert(err_copy.get_failure() == 42);

    // Move ctor leaves the new object holding the moved value.
    outcome_t ok_moved(std::move(ok_copy));
    assert(ok_moved.is_success());
    assert(ok_moved.get_result() == "good-payload");

    // rvalue-overload get_result returns by &&, so std::move on a temporary
    // outcome yields the inner string.
    std::string extracted = outcome_t(std::string("steal-me")).get_result();
    assert(extracted == "steal-me");

    return 0;
}
