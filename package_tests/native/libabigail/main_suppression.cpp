/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: suppression; calls abigail::suppr::read_suppressions(), abigail::suppr::is_function_suppression().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-suppression.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

int main(void) {
    /* The minimal shape of an .abignore function-suppression entry:
       a `[suppress_function]` section with a `name` property naming
       the symbol to drop from diff reports, plus a free-form `label`
       comment. */
    const std::string doc =
        "[suppress_function]\n"
        "label = demo-libabigail-smoke\n"
        "name = my_excluded_symbol\n";
    std::istringstream input(doc);

    abigail::suppr::suppressions_type suppressions;
    abigail::suppr::read_suppressions(input, suppressions);

    /* The single `[suppress_function]` section must yield exactly one
       parsed suppression. */
    assert(suppressions.size() == 1);
    abigail::suppr::suppression_sptr s = suppressions[0];
    assert(s);

    /* Base-class label getter (declared on `suppression_base`) must
       carry over the INI property verbatim. */
    assert(s->get_label() == "demo-libabigail-smoke");

    /* The polymorphic downcast helper must recognize this as a
       function suppression — i.e. the parser dispatched on the
       `[suppress_function]` section name correctly. */
    abigail::suppr::function_suppression_sptr fs =
        abigail::suppr::is_function_suppression(s);
    assert(fs);

    /* The function-suppression-specific `name` field must come from
       the INI `name = ...` property. */
    assert(fs->get_name() == "my_excluded_symbol");

    /* A different INI section name must NOT be misclassified as a
       function suppression — feed a [suppress_variable] section and
       verify the same suppression_sptr is_function_suppression()
       returns null. */
    {
        const std::string var_doc =
            "[suppress_variable]\n"
            "name = my_excluded_global\n";
        std::istringstream var_input(var_doc);
        abigail::suppr::suppressions_type var_suppressions;
        abigail::suppr::read_suppressions(var_input, var_suppressions);
        assert(var_suppressions.size() == 1);
        assert(!abigail::suppr::is_function_suppression(var_suppressions[0]));
    }

    std::cout << "libabigail suppression test OK: parsed "
              << suppressions.size() << " function suppression(s)"
              << std::endl;
    return 0;
}
