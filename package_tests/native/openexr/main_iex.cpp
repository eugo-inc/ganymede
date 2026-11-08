/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for openexr: iex; calls IEX_NAMESPACE::ArgExc(), IEX_NAMESPACE::LogicExc(), IEX_NAMESPACE::InputExc().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <IexBaseExc.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <string>

int main(void) {
    const char* msg = "demo openexr iex probe message";

    /* 1. Throw a derived class, catch by base-class reference — RTTI must work. */
    bool caught_as_base = false;
    try {
        throw IEX_NAMESPACE::ArgExc(msg);
    } catch (const IEX_NAMESPACE::BaseExc& e) {
        caught_as_base = true;
        /* what() must carry the message verbatim. */
        const char* w = e.what();
        assert(w != NULL);
        assert(std::strstr(w, msg) != NULL);
    }
    assert(caught_as_base);

    /* 2. The derived class must also be catchable by its exact type. */
    bool caught_as_derived = false;
    try {
        throw IEX_NAMESPACE::LogicExc(std::string(msg));
    } catch (const IEX_NAMESPACE::LogicExc& e) {
        caught_as_derived = true;
        assert(std::strstr(e.what(), msg) != NULL);
    }
    assert(caught_as_derived);

    /* 3. BaseExc IS-A std::exception — catch via the std root must work too. */
    bool caught_as_std = false;
    try {
        throw IEX_NAMESPACE::InputExc(msg);
    } catch (const std::exception& e) {
        caught_as_std = true;
        assert(std::strstr(e.what(), msg) != NULL);
    }
    assert(caught_as_std);

    std::cout << "openexr iex test OK" << std::endl;
    return 0;
}
