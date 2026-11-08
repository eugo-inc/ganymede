/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rdkit: core API; calls RDKit::SmilesToMol(), RDKit::MolToSmiles().
 */

#include <GraphMol/GraphMol.h>
#include <GraphMol/RWMol.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <cassert>
#include <cstdio>
#include <memory>
#include <string>

int main() {
    /* Parse benzene's aromatic SMILES — exercises libRDKitSmilesParse end-to-end. */
    std::unique_ptr<RDKit::RWMol> mol(RDKit::SmilesToMol("c1ccccc1"));
    assert(mol != nullptr);
    assert(mol->getNumAtoms() == 6);
    assert(mol->getNumBonds() == 6);

    std::string canonical = RDKit::MolToSmiles(*mol);
    assert(!canonical.empty());
    assert(canonical.find("c1ccccc1") != std::string::npos);

    std::printf("rdkit test OK: %s (%zu atoms, %zu bonds)\n",
                canonical.c_str(), mol->getNumAtoms(), mol->getNumBonds());
    return 0;
}
