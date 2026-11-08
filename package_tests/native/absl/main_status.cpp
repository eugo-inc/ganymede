/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: status; calls absl::InvalidArgumentError(), absl::OutOfRangeError(), absl::OkStatus().
 */

#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/string_view.h>
#include <cassert>
#include <cstdio>
#include <string>
#include <utility>

static absl::StatusOr<int> parse_positive(absl::string_view s) {
    if (s.empty()) {
        return absl::InvalidArgumentError("empty input");
    }
    int n = 0;
    for (char c : s) {
        if (c < '0' || c > '9') {
            return absl::InvalidArgumentError("non-digit");
        }
        n = n * 10 + (c - '0');
    }
    if (n == 0) {
        return absl::OutOfRangeError("must be positive");
    }
    return n;
}

int main() {
    /* (1) OkStatus must be ok + kOk + empty message. */
    const absl::Status ok = absl::OkStatus();
    assert(ok.ok());
    assert(ok.code() == absl::StatusCode::kOk);
    assert(ok.message().empty());

    /* (2) An error status preserves code + message; ToString prefixes with the code name. */
    const absl::Status bad = absl::InvalidArgumentError("bad thing happened");
    assert(!bad.ok());
    assert(bad.code() == absl::StatusCode::kInvalidArgument);
    assert(bad.message() == "bad thing happened");
    const std::string repr = bad.ToString();
    /* Conventional format is "INVALID_ARGUMENT: bad thing happened"; assert substrings to
     * tolerate trailing payload context without coupling to the exact representation. */
    assert(repr.find("INVALID_ARGUMENT") != std::string::npos);
    assert(repr.find("bad thing happened") != std::string::npos);

    /* (3) StatusOr<int> in error state. */
    absl::StatusOr<int> err = parse_positive("abc");
    assert(!err.ok());
    assert(err.status().code() == absl::StatusCode::kInvalidArgument);
    assert(err.status().message() == "non-digit");

    absl::StatusOr<int> empty_err = parse_positive("");
    assert(!empty_err.ok());
    assert(empty_err.status().code() == absl::StatusCode::kInvalidArgument);

    absl::StatusOr<int> zero_err = parse_positive("0");
    assert(!zero_err.ok());
    assert(zero_err.status().code() == absl::StatusCode::kOutOfRange);

    /* (4) StatusOr<int> in ok state: value() and operator* both return the value. */
    absl::StatusOr<int> ok_val = parse_positive("42");
    assert(ok_val.ok());
    assert(ok_val.status().ok());
    assert(*ok_val == 42);
    assert(ok_val.value() == 42);

    /* Larger value to exercise the digit-accumulation loop. */
    absl::StatusOr<int> ok_big = parse_positive("12345");
    assert(ok_big.ok());
    assert(*ok_big == 12345);

    std::printf("absl status test OK: parse_positive(\"42\")=%d\n", *ok_val);
    return 0;
}
