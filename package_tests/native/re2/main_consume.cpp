/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for re2: consume; calls RE2::Consume(), RE2::FindAndConsume().
 */

#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    /* Aspect A: Consume — strict left-anchored match that advances the cursor. */
    {
        std::string buf = "key1=42;key2=7;";
        re2::StringPiece input(buf);
        RE2 pair_re("(\\w+)=(\\d+);");

        std::string key;
        int value = 0;

        /* First Consume: matches "key1=42;", advances input. */
        assert(RE2::Consume(&input, pair_re, &key, &value));
        assert(key == "key1");
        assert(value == 42);

        /* Second Consume: matches "key2=7;", advances input to empty. */
        assert(RE2::Consume(&input, pair_re, &key, &value));
        assert(key == "key2");
        assert(value == 7);
        assert(input.empty());

        /* Third Consume on empty input: no match. */
        assert(!RE2::Consume(&input, pair_re, &key, &value));
    }

    /* Aspect B: Consume must FAIL if the pattern is not anchored at the start. */
    {
        re2::StringPiece input("noise key=42;");
        RE2 pair_re("(\\w+)=(\\d+);");
        std::string key;
        int value = 0;
        /* Consume requires the match to begin at offset 0; "noise " is in the
         * way, so this must return false even though the pattern would match
         * later in the string. */
        assert(!RE2::Consume(&input, pair_re, &key, &value));
        /* Cursor unchanged on failure. */
        assert(input == "noise key=42;");
    }

    /* Aspect C: FindAndConsume — scans ahead, then advances past the match. */
    {
        std::string buf = "garbage 1, more 2, even more 3 end";
        re2::StringPiece input(buf);
        RE2 num_re("(\\d+)");

        std::vector<int> found;
        int v = 0;
        while (RE2::FindAndConsume(&input, num_re, &v)) {
            found.push_back(v);
        }
        assert(found.size() == 3);
        assert(found[0] == 1);
        assert(found[1] == 2);
        assert(found[2] == 3);
        /* After exhausting matches, what remains is the tail text after "3". */
        assert(input == " end");
    }

    std::cout << "re2 consume test OK" << std::endl;
    return 0;
}
