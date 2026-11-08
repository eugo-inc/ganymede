/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for opentelemetry: resource; calls sdk_resource::Resource::Create(), nostd::holds_alternative(), nostd::get(), sdk_resource::Resource::GetEmpty().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <opentelemetry/version.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/common/attribute_value.h>
#include <opentelemetry/nostd/variant.h>
#include <cassert>
#include <cstdio>
#include <string>

namespace sdk_resource = opentelemetry::sdk::resource;
namespace nostd = opentelemetry::nostd;

int main() {
    sdk_resource::ResourceAttributes attributes = {
        {"service.name", "demo-smoke"},
        {"service.version", "1.0"},
    };
    auto resource = sdk_resource::Resource::Create(attributes);

    const auto& attrs = resource.GetAttributes();
    auto it = attrs.find("service.name");
    assert(it != attrs.end());

    /* AttributeValue is a variant; the "service.name" entry must hold a
     * string. Resource::Create() coerces const char* to std::string in the
     * stored variant. */
    const auto& service_name_value = it->second;
    assert(nostd::holds_alternative<std::string>(service_name_value));
    assert(nostd::get<std::string>(service_name_value) == "demo-smoke");

    auto empty = sdk_resource::Resource::GetEmpty();
    const auto& empty_attrs = empty.GetAttributes();
    assert(empty_attrs.find("service.name") == empty_attrs.end());

    sdk_resource::ResourceAttributes override_attrs = {
        {"service.name", "demo-smoke-override"},
        {"host.name", "demo-host"},
    };
    auto overrider = sdk_resource::Resource::Create(override_attrs);
    auto merged = resource.Merge(overrider);

    const auto& merged_attrs = merged.GetAttributes();
    auto merged_service = merged_attrs.find("service.name");
    assert(merged_service != merged_attrs.end());
    assert(nostd::holds_alternative<std::string>(merged_service->second));
    assert(nostd::get<std::string>(merged_service->second) == "demo-smoke-override");

    auto merged_host = merged_attrs.find("host.name");
    assert(merged_host != merged_attrs.end());
    assert(nostd::holds_alternative<std::string>(merged_host->second));
    assert(nostd::get<std::string>(merged_host->second) == "demo-host");

    /* The original service.version (only in the "self" Resource) survives. */
    auto merged_version = merged_attrs.find("service.version");
    assert(merged_version != merged_attrs.end());

    std::printf("opentelemetry resource test OK: %s\n", OPENTELEMETRY_VERSION);
    return 0;
}
