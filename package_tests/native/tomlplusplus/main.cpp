/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for tomlplusplus: core API; calls toml::parse().
 */

#include <toml++/toml.hpp>

#if TOML_HEADER_ONLY
#error "toml++ compiled header-only: this probe must use the installed libtomlplusplus.so (-DTOML_HEADER_ONLY=0 -DTOML_SHARED_LIB=1)"
#endif

#include <cstdint>
#include <cstdio>
#include <string_view>

int main() {
    using namespace std::string_view_literals;

    auto config = toml::parse("[server]\nport = 8000\nname = \"demo\"\n"sv);
    const auto port = config["server"]["port"].value<std::int64_t>();
    const auto name = config["server"]["name"].value<std::string_view>();
    if (!port || *port != 8000 || !name || *name != "demo"sv) {
        std::fprintf(stderr, "toml++ read back the wrong values\n");
        return 1;
    }

    std::printf("toml++ %d.%d.%d (library mode): parsed [server] port=%lld name=%.*s\n", TOML_LIB_MAJOR,
                TOML_LIB_MINOR, TOML_LIB_PATCH, static_cast<long long>(*port), static_cast<int>(name->size()),
                name->data());
    return 0;
}
