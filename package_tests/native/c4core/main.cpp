/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for c4core: core API; calls c4::atod(), c4::csubstr(), c4::cat(), c4::version().
 */

#include <c4/charconv.hpp>
#include <c4/format.hpp>
#include <c4/substr.hpp>
#include <c4/version.hpp>

#include <cstdio>
#include <cstring>

int main() {
  double value = 0.0;
  if (!c4::atod(c4::csubstr("2.718281828"), &value) || value != 2.718281828) {
    std::fprintf(stderr, "c4::atod failed: %.17g\n", value);
    return 1;
  }
  int integer = 0;
  if (!c4::atoi(c4::csubstr("-42"), &integer) || integer != -42) {
    std::fprintf(stderr, "c4::atoi failed: %d\n", integer);
    return 1;
  }
  char buffer[64] = {};
  c4::substr out(buffer, sizeof(buffer));
  const size_t written = c4::cat(out, "c4core ", c4::version(), " ok");
  if (written == 0 || written >= sizeof(buffer) || std::strncmp(buffer, "c4core ", 7) != 0) {
    std::fprintf(stderr, "c4::cat failed\n");
    return 1;
  }
  buffer[written] = '\0';
  std::printf("%s (atod 2.718281828, atoi -42)\n", buffer);
  return 0;
}
