/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for robin_map: hash; compiles against <tsl/robin_map.h>.
 */

#include <tsl/robin_map.h>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <string>

namespace {

struct CaseInsensitiveHash {
    std::size_t operator()(const std::string& s) const noexcept {
        std::size_t h = 1469598103934665603ULL; /* FNV-1a 64 offset */
        for (char c : s) {
            unsigned char lc = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
            h ^= static_cast<std::size_t>(lc);
            h *= 1099511628211ULL;
        }
        return h;
    }
};

struct CaseInsensitiveEq {
    bool operator()(const std::string& a, const std::string& b) const noexcept {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            unsigned char ca = static_cast<unsigned char>(a[i]);
            unsigned char cb = static_cast<unsigned char>(b[i]);
            if (std::tolower(ca) != std::tolower(cb)) return false;
        }
        return true;
    }
};

} /* namespace */

int main() {
    tsl::robin_map<std::string, int, CaseInsensitiveHash, CaseInsensitiveEq> m;
    m.insert({"Hello", 1});
    m["WORLD"] = 2;

    /* Lookups with differently-cased keys must hit the SAME slots. If the
     * custom Hash/KeyEqual were ignored, these would miss. */
    assert(m.size() == 2);
    assert(m.at("hello") == 1);
    assert(m.at("HELLO") == 1);
    assert(m.at("world") == 2);
    assert(m.at("World") == 2);

    /* Overwriting with a different case must NOT add a new entry. */
    m["hello"] = 11;
    assert(m.size() == 2);
    assert(m.at("HeLLo") == 11);

    /* count() must respect the custom equality too. */
    assert(m.count("HELLO") == 1);
    assert(m.count("missing") == 0);

    return 0;
}
