/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for xtl: base64; calls xtl::base64encode(), xtl::base64decode().
 */

#include <xtl/xbase64.hpp>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    const std::string plaintext = "foobar";
    const std::string expected_b64 = "Zm9vYmFy";

    const std::string encoded = xtl::base64encode(plaintext);
    assert(encoded == expected_b64);

    const std::string decoded = xtl::base64decode(encoded);
    assert(decoded == plaintext);

    const std::string longer = "demo-xtl-base64-test";
    const std::string round_trip = xtl::base64decode(xtl::base64encode(longer));
    assert(round_trip == longer);

    /* Empty input edge case. */
    const std::string empty_enc = xtl::base64encode(std::string{});
    const std::string empty_dec = xtl::base64decode(empty_enc);
    assert(empty_dec.empty());

    std::cout << "xtl xbase64 test OK" << std::endl;
    return 0;
}
