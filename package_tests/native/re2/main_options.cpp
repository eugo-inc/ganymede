/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for re2: options; calls RE2::FullMatch().
 */

#include <re2/re2.h>
#include <cassert>
#include <iostream>
#include <string>

int main() {
    /* Aspect A: case-insensitive Options. */
    {
        RE2::Options opts;
        opts.set_case_sensitive(false);
        RE2 re("hello", opts);
        assert(re.ok());
        assert(RE2::FullMatch("HELLO", re));
        assert(RE2::FullMatch("HeLLo", re));
        assert(!RE2::FullMatch("goodbye", re));
    }

    /* Aspect B: literal Options — regex metacharacters are matched verbatim. */
    {
        RE2::Options opts;
        opts.set_literal(true);
        RE2 re("a.b*c", opts);
        assert(re.ok());
        /* Without literal, "a.b*c" would match "aXc" / "abc" / etc. With
         * literal=true it ONLY matches the exact 5-byte string "a.b*c". */
        assert(RE2::FullMatch("a.b*c", re));
        assert(!RE2::FullMatch("aXbbbc", re));
        assert(!RE2::FullMatch("abc", re));
    }

    /* Aspect C: malformed pattern → !ok(), error_code() != NoError, error()
     * yields a non-empty diagnostic. Silence the noisy ERROR log first. */
    {
        RE2::Options opts;
        opts.set_log_errors(false);
        RE2 bad("(unclosed", opts);
        assert(!bad.ok());
        assert(bad.error_code() != RE2::NoError);
        assert(!bad.error().empty());
    }

    /* Aspect D: malformed escape — distinct ErrorCode path. */
    {
        RE2::Options opts;
        opts.set_log_errors(false);
        RE2 bad("\\q", opts);
        assert(!bad.ok());
        assert(bad.error_code() != RE2::NoError);
    }

    std::cout << "re2 options test OK" << std::endl;
    return 0;
}
