/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for folly: folly; compiles against <folly/Conv.h>, <folly/Portability.h>, <folly/container/F14Map.h>.
 */

#include <folly/Conv.h>
#include <folly/Portability.h>
#include <folly/container/F14Map.h>

#include <cstdio>
#include <string>

int main() {
  folly::F14FastMap<std::string, int> map;
  map["answer"] = folly::to<int>("42");
  const std::string text = folly::to<std::string>(map.at("answer"), "-", 7);
  if (text != "42-7") {
    std::printf("FAIL: folly::to produced '%s', want '42-7'\n", text.c_str());
    return 1;
  }
  if (!folly::kIsLinux) {
    std::printf("FAIL: folly-config.h/Portability.h say this is not Linux\n");
    return 1;
  }
  std::printf("folly ok: %s, F14 size %zu\n", text.c_str(), map.size());
  return 0;
}
