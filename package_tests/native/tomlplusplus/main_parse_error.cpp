/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tomlplusplus: parse error; calls toml::parse().
 */

#include <toml++/toml.hpp>

#if TOML_HEADER_ONLY
#error "toml++ compiled header-only: this probe must use the installed libtomlplusplus.so (-DTOML_HEADER_ONLY=0 -DTOML_SHARED_LIB=1)"
#endif

#if !TOML_EXCEPTIONS
#error "toml++ built without exceptions: toml::parse reports errors by return value, and this probe checks the throw"
#endif

#include <cstdio>
#include <exception>
#include <string_view>

int main() {
    using namespace std::string_view_literals;

    try {
        [[maybe_unused]] auto config = toml::parse("[server]\nport = = 8000\n"sv);
        std::fprintf(stderr, "toml++ accepted an invalid document (no exception thrown)\n");
        return 1;
    } catch (const std::exception& error) {
        const char* message = error.what();
        if (message == nullptr || message[0] == '\0') {
            std::fprintf(stderr, "toml++ rejected the invalid document but the exception has an empty message\n");
            return 1;
        }
        std::printf("toml++ rejected an invalid document: %s\n", message);
    }
    return 0;
}
