# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo
"""Functional check for rdkit: CoordGen layout and MAE round trip through the Python API, plus
InChI and DetermineBonds when this RDKit includes them.

Run by the rdkit spec's smoke_run with the prefix's Python. InChI and DetermineBonds are
optional RDKit features (a default source build leaves them out); when one is not built, a
``... not built, skipped`` note goes to stdout and the check carries on. Prints
``RDKIT-COORDGEN-MAEPARSER-CHECK: PASS`` on success; on failure it prints one
``RDKIT-COORDGEN-MAEPARSER-CHECK: FAIL <reason>`` line on stderr and exits 1.
"""

import importlib.util
import os
import sys
import tempfile
from typing import NoReturn


def fail(reason: str) -> NoReturn:
    print(f"RDKIT-COORDGEN-MAEPARSER-CHECK: FAIL {reason}", file=sys.stderr)
    sys.exit(1)


try:
    from rdkit import Chem
    from rdkit.Chem import rdCoordGen, rdDepictor
except Exception as error:
    fail(f"import rdkit.Chem / rdCoordGen failed: {error}")

if not rdDepictor.IsCoordGenSupportAvailable():
    fail("rdDepictor.IsCoordGenSupportAvailable() is False: this RDKit has no CoordGen support")

# CoordGen: lay out a 12-membered ring (the macrocycle code path); expect one conformer with finite coordinates.
ring = Chem.MolFromSmiles("C1CCCCCCCCCCC1")
rdCoordGen.AddCoords(ring)
if ring.GetNumConformers() != 1:
    fail(f"CoordGen produced {ring.GetNumConformers()} conformers for a 12-ring, expected 1")
if not all(abs(value) < 1e6 for point in ring.GetConformer().GetPositions() for value in point):
    fail("CoordGen produced a non-finite or absurd coordinate for a 12-ring")

# maeparser: write benzene to a MAE file and read it back; the same molecule must come back.
benzene = Chem.MolFromSmiles("c1ccccc1")
rdCoordGen.AddCoords(benzene)
with tempfile.TemporaryDirectory() as folder:
    path = os.path.join(folder, "benzene.mae")
    writer = Chem.MaeWriter(path)
    writer.write(benzene)
    writer.close()
    read_back = [molecule for molecule in Chem.MaeMolSupplier(path) if molecule is not None]
if len(read_back) != 1:
    fail(f"MAE round trip returned {len(read_back)} molecules, expected 1")
expected, got = Chem.MolToSmiles(benzene), Chem.MolToSmiles(read_back[0])
if expected != got:
    fail(f"MAE round trip changed the molecule: wrote {expected}, read back {got}")


def is_built(module: str) -> bool:
    """True when this RDKit ships the extension module, whether or not it then loads."""
    return importlib.util.find_spec(module) is not None


# InChI (optional): benzene's identifier, its hashed key, and the way back (PubChem CID 241).
if not is_built("rdkit.Chem.rdinchi"):
    print("InChI: rdkit.Chem.rdinchi not built, skipped")
else:
    try:
        from rdkit.Chem import inchi
    except Exception as error:
        fail(f"import rdkit.Chem.inchi failed: {error}")
    if not inchi.INCHI_AVAILABLE:
        fail("rdkit.Chem.rdinchi is present but did not load (rdkit.Chem.inchi.INCHI_AVAILABLE is False)")
    benzene_inchi = Chem.MolToInchi(Chem.MolFromSmiles("c1ccccc1"))
    if benzene_inchi != "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H":
        fail(f"the InChI of benzene is {benzene_inchi!r}, expected 'InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H'")
    benzene_key = Chem.InchiToInchiKey(benzene_inchi)
    if benzene_key != "UHOVQNZJYSORNB-UHFFFAOYSA-N":
        fail(f"the InChIKey of benzene is {benzene_key!r}, expected 'UHOVQNZJYSORNB-UHFFFAOYSA-N'")
    benzene_back = Chem.MolFromInchi(benzene_inchi)
    if benzene_back is None or Chem.MolToSmiles(benzene_back) != "c1ccccc1":
        fail("benzene did not come back from its InChI as c1ccccc1")

# DetermineBonds (optional): recover ethanol's bonds from bare 3D coordinates (an XYZ block).
ETHANOL_XYZ = """9

C     -0.8632    0.2204    0.2027
C      0.4063   -0.3750   -0.3700
O      1.5274    0.4036    0.0222
H     -1.7385   -0.3588   -0.1041
H     -0.9883    1.2554   -0.1319
H     -0.8211    0.2424    1.2967
H      0.5515   -1.4013   -0.0193
H      0.3645   -0.3865   -1.4630
H      1.5614    0.3998    0.9940
"""
if not is_built("rdkit.Chem.rdDetermineBonds"):
    print("DetermineBonds: rdkit.Chem.rdDetermineBonds not built, skipped")
else:
    try:
        from rdkit.Chem import rdDetermineBonds
    except Exception as error:
        fail(f"import rdkit.Chem.rdDetermineBonds failed: {error}")
    ethanol = Chem.MolFromXYZBlock(ETHANOL_XYZ)
    if ethanol is None:
        fail("Chem.MolFromXYZBlock could not read the ethanol XYZ block")
    rdDetermineBonds.DetermineBonds(ethanol, charge=0)
    ethanol_smiles = Chem.MolToSmiles(Chem.RemoveHs(ethanol))
    if ethanol_smiles != "CCO":
        fail(f"DetermineBonds on ethanol's coordinates gave {ethanol_smiles!r}, expected 'CCO'")

print("RDKIT-COORDGEN-MAEPARSER-CHECK: PASS")
