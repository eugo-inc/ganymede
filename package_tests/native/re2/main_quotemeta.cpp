/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for re2: quotemeta; calls RE2::QuoteMeta(), RE2::FullMatch().
 */

#include <re2/re2.h>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    /* Aspect A: QuoteMeta escapes regex metacharacters so the result is a
     * literal pattern. */
    {
        std::string escaped = RE2::QuoteMeta("a.b*c+d?");
        RE2 lit(escaped);
        assert(lit.ok());
        assert(RE2::FullMatch("a.b*c+d?", lit));
        assert(!RE2::FullMatch("aXbcd", lit));
        assert(!RE2::FullMatch("abbbbcd", lit));
    }

    /* Aspect B: QuoteMeta on a string with NO metacharacters is a no-op. */
    {
        std::string escaped = RE2::QuoteMeta("plain_text_123");
        assert(escaped == "plain_text_123");
    }

    /* Aspect C: NumberOfCapturingGroups + pattern() introspection. */
    {
        RE2 re("(\\w+)-(\\d+)-(\\w+)");
        assert(re.ok());
        assert(re.NumberOfCapturingGroups() == 3);
        assert(re.pattern() == "(\\w+)-(\\d+)-(\\w+)");

        /* A pattern with NO capture groups reports 0. */
        RE2 nocap("\\w+");
        assert(nocap.NumberOfCapturingGroups() == 0);
    }

    /* Aspect D: typed captures into long long, unsigned int and double. */
    {
        RE2 re("(-?\\d+)\\s+(\\d+)\\s+([0-9.]+)");
        assert(re.ok());

        long long big = 0;
        unsigned int u = 0;
        double d = 0.0;
        assert(RE2::FullMatch("-9000000000 4242 3.14", re, &big, &u, &d));
        assert(big == -9000000000LL);
        assert(u == 4242u);
        /* Float compare with a tolerance — re2 parses via strtod. */
        assert(d > 3.139 && d < 3.141);
    }

    /* Aspect E: Arg adapter type-mismatch — feeding a non-numeric capture
     * into an int& must fail the match (Arg::Parse returns false). */
    {
        RE2 re("(\\w+)");
        int v = 0;
        /* The capture is the literal word "abc" — cannot parse as int. */
        assert(!RE2::FullMatch("abc", re, &v));
    }

    std::cout << "re2 quotemeta test OK" << std::endl;
    return 0;
}
