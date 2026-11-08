/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for maeparser: reads a two-atom MAE block with schrodinger::mae::Reader, writes it with Writer and reads it back.
 */

#include <maeparser/Reader.hpp>
#include <maeparser/Writer.hpp>
#include <maeparser/MaeBlock.hpp>
#include <cstdio>
#include <memory>
#include <sstream>
static const char* kMae =
  "{\n  s_m_m2io_version\n  :::\n  2.0.0\n}\n\n"
  "f_m_ct {\n  s_m_title\n  :::\n  ethane_fragment\n"
  "  m_atom[2] {\n    # First column is atom index #\n    r_m_x_coord\n    r_m_y_coord\n    r_m_z_coord\n    i_m_atomic_number\n    :::\n"
  "    1 0.0 0.0 0.0 6\n    2 1.54 0.0 0.0 6\n    :::\n  }\n}\n";
static int check(const std::shared_ptr<schrodinger::mae::Block>& ct, const char* when) {
    if (!ct) { std::printf("%s: no f_m_ct block\n", when); return 1; }
    auto atoms = ct->getIndexedBlock("m_atom");
    auto z = atoms->getIntProperty("i_m_atomic_number");
    auto x = atoms->getRealProperty("r_m_x_coord");
    std::printf("%s: title=%s atoms=%zu z0=%d x1=%.2f\n", when, ct->getStringProperty("s_m_title").c_str(), atoms->size(), z->at(0), x->at(1));
    return (ct->getStringProperty("s_m_title") == "ethane_fragment" && atoms->size() == 2 && z->at(0) == 6 && x->at(1) > 1.53 && x->at(1) < 1.55) ? 0 : 1;
}
int main() {
    auto in = std::make_shared<std::stringstream>(kMae);
    schrodinger::mae::Reader r(in);
    auto ct = r.next("f_m_ct");
    int rc = check(ct, "read");
    auto out = std::make_shared<std::stringstream>();
    { schrodinger::mae::Writer w(out); w.write(ct); }
    auto again = std::make_shared<std::stringstream>(out->str());
    schrodinger::mae::Reader r2(again);
    return rc | check(r2.next("f_m_ct"), "round-trip");
}
