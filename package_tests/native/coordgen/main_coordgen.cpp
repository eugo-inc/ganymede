/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for coordgen: coordgen; calls sketcherMinimizerMolecule().
 */

#include <coordgen/sketcherMinimizer.h>
#include <cmath>
#include <cstdio>
#include <vector>
static int ring(int n, int alternate_double) {
    sketcherMinimizer minimizer;
    auto* mol = new sketcherMinimizerMolecule();
    std::vector<sketcherMinimizerAtom*> atoms;
    for (int i = 0; i < n; ++i) { auto a = mol->addNewAtom(); a->setAtomicNumber(6); atoms.push_back(a); }
    for (int i = 0; i < n; ++i) { auto b = mol->addNewBond(atoms[i], atoms[(i + 1) % n]); b->setBondOrder(alternate_double && (i % 2) ? 2 : 1); }
    minimizer.initialize(mol);
    if (!minimizer.runGenerateCoordinates()) { std::printf("ring%d: runGenerateCoordinates returned false\n", n); return 1; }
    double lo = 1e9, hi = 0;
    for (int i = 0; i < n; ++i) {
        auto d = atoms[i]->getCoordinates() - atoms[(i + 1) % n]->getCoordinates();
        double len = std::sqrt(d.x() * d.x() + d.y() * d.y());
        if (!std::isfinite(len)) { std::printf("ring%d: non-finite coordinate\n", n); return 1; }
        lo = std::fmin(lo, len); hi = std::fmax(hi, len);
    }
    std::printf("ring%d: bond length min=%.3f max=%.3f\n", n, lo, hi);
    return (lo > 40.0 && hi < 60.0) ? 0 : 1;   // coordgen draws single bonds at ~50 units (RDKit CoordGen.h:32-34)
}
int main() { return ring(6, 1) | ring(12, 0); }
