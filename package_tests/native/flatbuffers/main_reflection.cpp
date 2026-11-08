/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for flatbuffers: reflection; calls reflection::GetSchema().
 */

#include <flatbuffers/idl.h>
#include <flatbuffers/reflection.h>
#include <flatbuffers/reflection_generated.h>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

int main() {
    /* Minimal schema with two scalar fields and one string field. */
    const std::string schema_src =
        "table Monster {\n"
        "  hp: int = 100;\n"
        "  mana: short = 150;\n"
        "  name: string;\n"
        "}\n"
        "root_type Monster;\n";

    flatbuffers::Parser parser;
    bool parsed = parser.Parse(schema_src.c_str());
    assert(parsed && "Parser rejected schema");

    /* Serialize the parsed schema to a binary .bfbs. This is the symbol that
     * REQUIRES the libflatbuffers .so/.a to be linked. */
    parser.Serialize();
    const uint8_t *bfbs = parser.builder_.GetBufferPointer();
    size_t bfbs_size = parser.builder_.GetSize();
    assert(bfbs != nullptr);
    assert(bfbs_size > 0);

    /* Deserialize the .bfbs into a reflection::Schema. */
    const reflection::Schema *schema = reflection::GetSchema(bfbs);
    assert(schema != nullptr);
    assert(schema->objects() != nullptr);
    assert(schema->objects()->size() >= 1);

    const reflection::Object *monster = nullptr;
    for (flatbuffers::uoffset_t i = 0; i < schema->objects()->size(); ++i) {
        const reflection::Object *obj = schema->objects()->Get(i);
        assert(obj != nullptr);
        assert(obj->name() != nullptr);
        if (std::strstr(obj->name()->c_str(), "Monster") != nullptr) {
            monster = obj;
            break;
        }
    }
    assert(monster != nullptr && "Monster table missing from reflected schema");
    assert(monster->fields() != nullptr);
    assert(monster->fields()->size() == 3);

    bool saw_hp = false, saw_mana = false, saw_name = false;
    for (flatbuffers::uoffset_t i = 0; i < monster->fields()->size(); ++i) {
        const reflection::Field *f = monster->fields()->Get(i);
        assert(f != nullptr);
        assert(f->name() != nullptr);
        assert(f->type() != nullptr);
        const char *fname = f->name()->c_str();
        reflection::BaseType bt = f->type()->base_type();
        if (std::strcmp(fname, "hp") == 0) {
            assert(bt == reflection::Int);
            saw_hp = true;
        } else if (std::strcmp(fname, "mana") == 0) {
            assert(bt == reflection::Short);
            saw_mana = true;
        } else if (std::strcmp(fname, "name") == 0) {
            assert(bt == reflection::String);
            saw_name = true;
        }
    }
    assert(saw_hp && saw_mana && saw_name && "reflection missed at least one field");

    std::printf("flatbuffers reflection test OK: %u objects, 3/3 Monster fields\n",
                schema->objects()->size());
    return 0;
}
