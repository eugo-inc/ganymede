/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: ini; calls abigail::ini::read_config().
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-ini.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

int main(void) {
    /* A two-section INI doc with a single string property in the second
       section — the minimal shape of an .abignore suppression file. */
    const std::string doc =
        "[first_section]\n"
        "name = libabigail-test\n"
        "\n"
        "[suppress_function]\n"
        "name = my_excluded_symbol\n";

    std::istringstream input(doc);
    abigail::ini::config_sptr conf = abigail::ini::read_config(input);
    assert(conf);

    const abigail::ini::config::sections_type& sections = conf->get_sections();
    assert(sections.size() == 2);

    /* Section order must be preserved. */
    assert(sections[0]->get_name() == "first_section");
    assert(sections[1]->get_name() == "suppress_function");

    /* Property lookup by name on the second section returns a real handle. */
    abigail::ini::property_sptr prop =
        sections[1]->find_property("name");
    assert(prop);
    assert(prop->get_name() == "name");

    /* A property that does NOT exist returns a null shared_ptr (not a throw). */
    abigail::ini::property_sptr missing =
        sections[1]->find_property("does_not_exist");
    assert(!missing);

    std::cout << "libabigail ini test OK: " << sections.size()
              << " sections parsed" << std::endl;
    return 0;
}
