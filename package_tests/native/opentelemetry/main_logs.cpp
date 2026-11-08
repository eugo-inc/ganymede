/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opentelemetry: logs; calls logs_api::Provider::GetLoggerProvider().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/logs/logger_provider.h>
#include <opentelemetry/logs/logger.h>
#include <opentelemetry/logs/log_record.h>
#include <opentelemetry/logs/severity.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/nostd/unique_ptr.h>
#include <cassert>
#include <cstdio>

namespace logs_api = opentelemetry::logs;
namespace nostd = opentelemetry::nostd;

int main() {
    /* Documented contract: GetLoggerProvider() never returns nullptr (default no-op). */
    nostd::shared_ptr<logs_api::LoggerProvider> provider = logs_api::Provider::GetLoggerProvider();
    assert(provider);

    nostd::shared_ptr<logs_api::Logger> logger = provider->GetLogger("demo-smoke", "demo-smoke-lib", "1.0");
    assert(logger);

    assert(static_cast<int>(logs_api::Severity::kInfo) > static_cast<int>(logs_api::Severity::kDebug));
    assert(static_cast<int>(logs_api::Severity::kError) > static_cast<int>(logs_api::Severity::kWarn));

    /* EmitLogRecord no-op path — vtable dispatch through Logger. CreateLogRecord
     * yields a unique_ptr<LogRecord>; the no-op logger swallows it without I/O. */
    nostd::unique_ptr<logs_api::LogRecord> rec = logger->CreateLogRecord();
    assert(rec);
    rec->SetSeverity(logs_api::Severity::kInfo);
    rec->SetBody("demo-smoke-log-body");
    logger->EmitLogRecord(std::move(rec));

    std::printf("opentelemetry logs test OK: %s\n", OPENTELEMETRY_VERSION);
    return 0;
}
