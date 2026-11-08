/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: regex; calls abigail::regex::compile(), abigail::regex::match(), abigail::regex::escape().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-regex.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

int main(void) {
    abigail::regex::regex_t_sptr re =
        abigail::regex::compile("^_Z[NK]?[0-9]+abigail.*$");
    assert(re);

    /* A C++ mangled-looking symbol starting with _ZN13abigail... matches. */
    assert(abigail::regex::match(re, "_ZN13abigail_internal4funcEv"));

    /* A plain C symbol must NOT match the same pattern. */
    assert(!abigail::regex::match(re, "main"));
    assert(!abigail::regex::match(re, "printf"));

    /* The `escape` helper streams a regex-escaped form of an input string;
       feeding a name with regex metacharacters and recompiling must work. */
    {
        std::ostringstream escaped;
        escaped << abigail::regex::escape(std::string("foo.bar+baz"));
        const std::string pat = escaped.str();
        /* The escape must have neutralized the metacharacters — the produced
           pattern must be strictly longer than the raw input. */
        assert(pat.size() > std::string("foo.bar+baz").size());

        abigail::regex::regex_t_sptr re2 = abigail::regex::compile(pat);
        assert(re2);
        /* The escaped pattern matches the original literal string exactly. */
        assert(abigail::regex::match(re2, "foo.bar+baz"));
        /* And does NOT match a string the unescaped regex would have matched
           (e.g. "fooXbarZbaz" where '.' would have matched 'X'). */
        assert(!abigail::regex::match(re2, "fooXbarZbaz"));
    }

    std::cout << "libabigail regex test OK" << std::endl;
    return 0;
}
