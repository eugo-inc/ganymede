/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for absl: time; calls absl::Minutes(), absl::Seconds(), absl::Hours(), absl::ToInt64Seconds().
 */

#include <absl/time/civil_time.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>
#include <cassert>
#include <cstdio>
#include <string>

int main() {
    /* (1) Duration arithmetic. */
    const absl::Duration one_minute = absl::Minutes(1);
    const absl::Duration sixty_seconds = absl::Seconds(60);
    assert(one_minute == sixty_seconds);

    const absl::Duration one_day = absl::Hours(24);
    assert(absl::ToInt64Seconds(one_day) == 86400);
    assert(absl::ToInt64Milliseconds(one_day) == 86400LL * 1000LL);

    /* Duration subtraction sanity. */
    const absl::Duration delta = absl::Hours(2) - absl::Minutes(30);
    assert(absl::ToInt64Minutes(delta) == 90);

    /* (2) Time + Duration round-trip via Unix seconds. */
    const int64_t unix_s = 1700000000;
    const absl::Time t = absl::FromUnixSeconds(unix_s);
    assert(absl::ToUnixSeconds(t) == unix_s);

    const absl::Time t_plus = t + absl::Hours(1);
    assert(absl::ToUnixSeconds(t_plus) == unix_s + 3600);

    /* (3) CivilSecond -> Time -> FormatTime in UTC. */
    const absl::TimeZone utc = absl::UTCTimeZone();
    const absl::CivilSecond cs(2024, 6, 15, 12, 34, 56);
    const absl::Time t_from_cs = absl::FromCivil(cs, utc);

    /* RFC3339 default format in UTC. */
    const std::string formatted = absl::FormatTime(absl::RFC3339_full, t_from_cs, utc);
    assert(formatted == "2024-06-15T12:34:56+00:00");

    const absl::CivilSecond cs_back = absl::ToCivilSecond(t_from_cs, utc);
    assert(cs_back.year() == 2024);
    assert(cs_back.month() == 6);
    assert(cs_back.day() == 15);
    assert(cs_back.hour() == 12);
    assert(cs_back.minute() == 34);
    assert(cs_back.second() == 56);

    std::printf("absl time test OK: %s\n", formatted.c_str());
    return 0;
}
