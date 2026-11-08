/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for spdlog: rotating file; compiles against <spdlog/spdlog.h>, <spdlog/sinks/rotating_file_sink.h>, <fstream>.
 *
 * Compiled header-only against an external fmt (-DSPDLOG_FMT_EXTERNAL), so it needs fmt installed
 * and does not exercise libspdlog.so itself.
 */

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>

int main(void) {
    /* Pick a unique temp path so concurrent test runs don't collide. */
    char path[256];
    std::snprintf(path, sizeof(path),
                  "/tmp/demo_spdlog_rot_%d.log", static_cast<int>(getpid()));

    auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        path, /*max_size=*/1024 * 64, /*max_files=*/3);
    auto logger = std::make_shared<spdlog::logger>("demo_rot", sink);
    logger->set_pattern("[%n] [%l] %v");
    logger->set_level(spdlog::level::info);

    logger->info("rot payload {}", 7);
    logger->flush();

    /* Read the file back and confirm content. */
    std::ifstream in(path);
    assert(in.is_open() && "rotating_file_sink did not create the log file");
    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string contents = buf.str();
    in.close();

    assert(contents.find("demo_rot") != std::string::npos);
    assert(contents.find("info") != std::string::npos);
    assert(contents.find("rot payload 7") != std::string::npos);

    /* Cleanup. */
    std::remove(path);

    std::printf("spdlog rotating_file test OK\n");
    return 0;
}
