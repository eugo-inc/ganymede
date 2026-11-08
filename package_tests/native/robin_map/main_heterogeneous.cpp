/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for robin_map: heterogeneous; compiles against <tsl/robin_map.h>.
 */

#include <tsl/robin_map.h>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

namespace {

struct TransparentHash {
    using is_transparent = void;

    static std::size_t fnv(const char* data, std::size_t len) noexcept {
        std::size_t h = 1469598103934665603ULL;
        for (std::size_t i = 0; i < len; ++i) {
            h ^= static_cast<unsigned char>(data[i]);
            h *= 1099511628211ULL;
        }
        return h;
    }

    std::size_t operator()(const std::string& s) const noexcept {
        return fnv(s.data(), s.size());
    }
    std::size_t operator()(const char* s) const noexcept {
        return fnv(s, std::strlen(s));
    }
};

/* Transparent equality — compares std::string, const char*, or each other. */
struct TransparentEq {
    using is_transparent = void;

    bool operator()(const std::string& a, const std::string& b) const noexcept {
        return a == b;
    }
    bool operator()(const std::string& a, const char* b) const noexcept {
        return a == b;
    }
    bool operator()(const char* a, const std::string& b) const noexcept {
        return b == a;
    }
    bool operator()(const char* a, const char* b) const noexcept {
        return std::strcmp(a, b) == 0;
    }
};

} /* namespace */

int main() {
    tsl::robin_map<std::string, int, TransparentHash, TransparentEq> m;
    m.insert({std::string("apple"), 1});
    m.insert({std::string("banana"), 2});
    m.insert({std::string("cherry"), 3});
    assert(m.size() == 3);

    /* (1) Heterogeneous find() with a const char* key. */
    const char* needle = "banana";
    auto it = m.find(needle);
    assert(it != m.end());
    assert(it->first == "banana");
    assert(it->second == 2);

    /* (2) Heterogeneous count. */
    assert(m.count("apple") == 1);
    assert(m.count("cherry") == 1);
    assert(m.count("missing") == 0);

    /* (3) Heterogeneous at(). */
    assert(m.at("apple") == 1);
    assert(m.at("cherry") == 3);

    /* (4) Heterogeneous erase via const char*. */
    auto erased = m.erase("banana");
    assert(erased == 1);
    assert(m.size() == 2);
    assert(m.find("banana") == m.end());

    /* (5) Negative control: erase of a missing heterogeneous key returns 0. */
    assert(m.erase("not-present") == 0);
    assert(m.size() == 2);

    return 0;
}
