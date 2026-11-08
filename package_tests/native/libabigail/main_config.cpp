/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libabigail: config; compiles against <libabigail/abg-config.h>.
 */

/* The checks below are assert() calls: keep them active even under -DNDEBUG. */
#undef NDEBUG

#include <libabigail/abg-config.h>
#include <cassert>
#include <iostream>
#include <string>

int main(void) {
    abigail::config conf;

    /* Documented constructor defaults — verbatim from abg-config.cc. */
    assert(conf.get_xml_element_indent() == 2);
    assert(conf.get_tu_instr_suffix() == ".bi");
    assert(conf.get_tu_instr_archive_suffix() == ".abi");

    /* The format-version defaults are seeded from the
       ABIGAIL_ABIXML_VERSION_{MAJOR,MINOR} compile-time macros — both
       must be non-empty digit-strings (same shape main_version.c
       verifies at the macro level). */
    const std::string& major = conf.get_format_major_version_number();
    const std::string& minor = conf.get_format_minor_version_number();
    assert(!major.empty());
    assert(!minor.empty());

    conf.set_xml_element_indent(7);
    assert(conf.get_xml_element_indent() == 7);

    conf.set_tu_instr_suffix(".demo-bi");
    assert(conf.get_tu_instr_suffix() == ".demo-bi");

    conf.set_tu_instr_archive_suffix(".demo-abi");
    assert(conf.get_tu_instr_archive_suffix() == ".demo-abi");

    conf.set_format_major_version_number("42");
    conf.set_format_minor_version_number("17");
    assert(conf.get_format_major_version_number() == "42");
    assert(conf.get_format_minor_version_number() == "17");

    std::cout << "libabigail config test OK: defaults indent=2 "
              << "abixml=" << major << "." << minor << std::endl;
    return 0;
}
