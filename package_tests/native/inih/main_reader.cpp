/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for inih: reader; compiles against <INIReader.h>.
 */

#include <INIReader.h>

#include <cstdio>
#include <string>
#include <vector>

static int failed = 0;

static void expect(const char *what, bool ok, const std::string &got) {
    if (!ok) {
        std::fprintf(stderr, "INIReader: %s: got '%s'\n", what, got.c_str());
        failed = 1;
    }
}

int main() {
    static const char doc[] = "[Server]\nPort = 0x1F90\nverbose = Yes\nratio = 0.5\nname = demo ; comment\n"
                              "[alpha]\nx = 1\n";
    const INIReader reader(doc, sizeof doc - 1);

    expect("ParseError", reader.ParseError() == 0, std::to_string(reader.ParseError()));
    expect("GetInteger (hex, case-insensitive key)", reader.GetInteger("server", "port", -1) == 8080,
           std::to_string(reader.GetInteger("server", "port", -1)));
    expect("GetBoolean", reader.GetBoolean("server", "verbose", false), "false");
    expect("GetReal", reader.GetReal("server", "ratio", 0.0) == 0.5,
           std::to_string(reader.GetReal("server", "ratio", 0.0)));
    expect("Get (inline comment stripped)", reader.Get("server", "name", "") == "demo",
           reader.Get("server", "name", ""));
    expect("HasValue (missing name)", !reader.HasValue("server", "missing"), "true");

    std::string sections;
    for (const std::string &section : reader.Sections()) {
        sections += section + ",";
    }
    expect("Sections (lower-cased, alphabetical)", sections == "alpha,server,", sections);

    static const char bad[] = "x = 1\nnovalue\n";
    const INIReader broken(bad, sizeof bad - 1);
    expect("ParseError on a name without a value", broken.ParseError() == 2, std::to_string(broken.ParseError()));

    const INIReader missing(std::string("/nonexistent/demo-inireader-probe.ini"));
    expect("missing file", missing.ParseError() == -1 && missing.ParseErrorMessage() == "unable to open file",
           missing.ParseErrorMessage());

    if (failed) {
        return 1;
    }
    std::printf("INIReader: buffer and file constructors, Get/GetInteger/GetBoolean/GetReal/Sections/HasValue OK\n");
    return 0;
}
