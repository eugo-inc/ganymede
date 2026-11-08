/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for jitify: reflect; calls reflection::reflect(), reflection::reflect_template().
 */

#include <jitify.hpp>

#include <cstdio>
#include <string>

namespace {

int check(const char* what, const std::string& got, const std::string& want, int code) {
    if (got != want) {
        std::fprintf(stderr, "FAIL: %s = \"%s\", expected \"%s\"\n", what, got.c_str(), want.c_str());
        return code;
    }
    return 0;
}

} // namespace

int main() {
    namespace reflection = jitify::reflection;

    if (int rc = check("reflect<float>()", reflection::reflect<float>(), "float", 2)) return rc;
    if (int rc = check("reflect<int, 7>()", reflection::reflect<int, 7>(), "(int)7", 3)) return rc;
    if (int rc = check("reflect_template<float, int>()", reflection::reflect_template<float, int>(), "<float,int >", 4))
        return rc;

    const std::string value = reflection::reflect(3.14f);
    if (value.rfind("(float)", 0) != 0) {
        std::fprintf(stderr, "FAIL: reflect(3.14f) = \"%s\", expected a \"(float)\" prefix\n", value.c_str());
        return 5;
    }

    std::printf("jitify OK: reflect<float>()=%s reflect<int,7>()=%s reflect_template<float,int>()=%s reflect(3.14f)=%s\n",
                reflection::reflect<float>().c_str(), reflection::reflect<int, 7>().c_str(),
                reflection::reflect_template<float, int>().c_str(), value.c_str());
    return 0;
}
