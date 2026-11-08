/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for spdlog: levels; compiles against <spdlog/spdlog.h>, <spdlog/sinks/ostream_sink.h>.
 *
 * Compiled header-only against an external fmt (-DSPDLOG_FMT_EXTERNAL), so it needs fmt installed
 * and does not exercise libspdlog.so itself.
 */

#include <spdlog/spdlog.h>
#include <spdlog/sinks/ostream_sink.h>

#include <cassert>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

int main(void) {
    std::ostringstream oss;
    auto sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    auto logger = std::make_shared<spdlog::logger>("demo_levels", sink);
    logger->set_pattern("%l|%v");
    logger->set_level(spdlog::level::warn);

    /* These should be filtered out. */
    logger->trace("trace_should_be_filtered");
    logger->debug("debug_should_be_filtered");
    logger->info("info_should_be_filtered");

    /* These should pass. */
    logger->warn("warn_should_pass");
    logger->error("error_should_pass");
    logger->critical("critical_should_pass");

    logger->flush();
    const std::string out = oss.str();

    /* Filtered messages must not appear. */
    assert(out.find("trace_should_be_filtered") == std::string::npos);
    assert(out.find("debug_should_be_filtered") == std::string::npos);
    assert(out.find("info_should_be_filtered") == std::string::npos);

    /* Passed messages must appear. */
    assert(out.find("warn_should_pass") != std::string::npos);
    assert(out.find("error_should_pass") != std::string::npos);
    assert(out.find("critical_should_pass") != std::string::npos);

    assert(!logger->should_log(spdlog::level::trace));
    assert(!logger->should_log(spdlog::level::debug));
    assert(!logger->should_log(spdlog::level::info));
    assert(logger->should_log(spdlog::level::warn));
    assert(logger->should_log(spdlog::level::err));
    assert(logger->should_log(spdlog::level::critical));

    std::printf("spdlog level-filter test OK\n");
    return 0;
}
