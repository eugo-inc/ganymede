/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for inchi: inchi; calls GetINCHI(), FreeINCHI(), MakeINCHIFromMolfileText(), GetINCHIKeyFromINCHI().
 */

#include <inchi/inchi_api.h>
#include <stdio.h>
#include <string.h>

static const char *const kBenzeneInchi = "InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H";
static const char *const kBenzeneKey = "UHOVQNZJYSORNB-UHFFFAOYSA-N";
static const char *const kAlanineInchi = "InChI=1S/C3H7NO2/c1-2(4)3(5)6/h2H,4H2,1H3,(H,5,6)/t2-/m0/s1";
static const char *const kAlanineKey = "QNAYBMKLOCPYGJ-REOHCLBHSA-N";

/* Benzene as a V2000 molfile: six carbons in a ring, alternating single and double bonds, hydrogens implicit. */
static const char *const kBenzeneMolfile =
    "benzene\n"
    "  demo-test\n"
    "\n"
    "  6  6  0  0  0  0  0  0  0  0999 V2000\n"
    "    1.2990    0.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000    1.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.2990    0.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "   -1.2990   -0.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    0.0000   -1.5000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "    1.2990   -0.7500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
    "  1  2  2  0\n"
    "  2  3  1  0\n"
    "  3  4  2  0\n"
    "  4  5  1  0\n"
    "  5  6  2  0\n"
    "  6  1  1  0\n"
    "M  END\n";

static int check_string(const char *what, const char *got, const char *want) {
    int ok = got != NULL && strcmp(got, want) == 0;
    printf("%s: %s (got \"%s\", want \"%s\")\n", what, ok ? "OK" : "FAIL", got ? got : "(null)", want);
    return ok ? 0 : 1;
}

static int check_built_benzene(void) {
    inchi_Atom atoms[6];
    memset(atoms, 0, sizeof atoms);
    for (int i = 0; i < 6; ++i) {
        strcpy(atoms[i].elname, "C");
        atoms[i].num_iso_H[0] = -1; /* let InChI add the implicit hydrogens */
    }
    /* Each bond is listed once, on its lower-numbered atom; ring closure 0-5 is single. */
    for (int i = 0; i < 5; ++i) {
        atoms[i].neighbor[atoms[i].num_bonds] = (AT_NUM)(i + 1);
        atoms[i].bond_type[atoms[i].num_bonds] = (i % 2 == 0) ? INCHI_BOND_TYPE_DOUBLE : INCHI_BOND_TYPE_SINGLE;
        atoms[i].num_bonds++;
    }
    atoms[0].neighbor[atoms[0].num_bonds] = 5;
    atoms[0].bond_type[atoms[0].num_bonds] = INCHI_BOND_TYPE_SINGLE;
    atoms[0].num_bonds++;

    inchi_Input input;
    memset(&input, 0, sizeof input);
    input.atom = atoms;
    input.num_atoms = 6;
    input.szOptions = "";

    inchi_Output output;
    memset(&output, 0, sizeof output);
    int rc = GetINCHI(&input, &output);
    int failed = (rc != inchi_Ret_OKAY && rc != inchi_Ret_WARNING);
    printf("GetINCHI(built benzene) returned %d\n", rc);
    failed |= check_string("built benzene InChI", output.szInChI, kBenzeneInchi);
    FreeINCHI(&output);
    return failed;
}

/* MakeINCHIFromMolfileText on a benzene molfile: the route RDKit's Chem.MolBlockToInchi takes. */
static int check_molfile_benzene(void) {
    inchi_Output output;
    memset(&output, 0, sizeof output);
    char options[] = "";
    int rc = MakeINCHIFromMolfileText(kBenzeneMolfile, options, &output);
    int failed = (rc != mol2inchi_Ret_OKAY && rc != mol2inchi_Ret_WARNING);
    printf("MakeINCHIFromMolfileText(benzene) returned %d\n", rc);
    failed |= check_string("molfile benzene InChI", output.szInChI, kBenzeneInchi);
    FreeINCHI(&output);
    return failed;
}

/* GetINCHIKeyFromINCHI: the route RDKit's Chem.MolToInchiKey takes (it computes the InChI first, then this). */
static int check_keys(void) {
    int failed = 0;
    char key[32];
    char extra1[80];
    char extra2[80];

    memset(key, 0, sizeof key);
    int rc = GetINCHIKeyFromINCHI(kBenzeneInchi, 0, 0, key, extra1, extra2);
    printf("GetINCHIKeyFromINCHI(benzene) returned %d\n", rc);
    failed |= (rc != INCHIKEY_OK) | check_string("benzene InChIKey", key, kBenzeneKey);

    memset(key, 0, sizeof key);
    rc = GetINCHIKeyFromINCHI(kAlanineInchi, 0, 0, key, extra1, extra2);
    printf("GetINCHIKeyFromINCHI(L-alanine) returned %d\n", rc);
    failed |= (rc != INCHIKEY_OK) | check_string("L-alanine InChIKey", key, kAlanineKey);
    return failed;
}

static int check_struct_from_alanine(void) {
    inchi_InputINCHI input;
    memset(&input, 0, sizeof input);
    input.szInChI = (char *)kAlanineInchi;
    input.szOptions = "";

    inchi_OutputStruct output;
    memset(&output, 0, sizeof output);
    int rc = GetStructFromINCHI(&input, &output);
    int heavy_atoms = 0;
    for (int i = 0; i < output.num_atoms; ++i) {
        heavy_atoms += strcmp(output.atom[i].elname, "H") != 0;
    }
    int centre_is_carbon = output.num_stereo0D == 1 && output.stereo0D[0].central_atom >= 0 &&
                           output.stereo0D[0].central_atom < output.num_atoms &&
                           strcmp(output.atom[output.stereo0D[0].central_atom].elname, "C") == 0;
    int ok = (rc == inchi_Ret_OKAY || rc == inchi_Ret_WARNING) && heavy_atoms == 6 && centre_is_carbon;
    printf("GetStructFromINCHI(L-alanine) returned %d: %d atoms (%d non-hydrogen), %d stereo elements: %s\n", rc,
           (int)output.num_atoms, heavy_atoms, (int)output.num_stereo0D,
           ok ? "OK" : "FAIL (want 6 non-hydrogen atoms and 1 stereo element on a carbon)");
    FreeStructFromINCHI(&output);
    return ok ? 0 : 1;
}

int main(void) {
    int failed = 0;
    failed |= check_built_benzene();
    failed |= check_molfile_benzene();
    failed |= check_keys();
    failed |= check_struct_from_alanine();
    printf("%s\n", failed ? "SOME CHECKS FAILED" : "ALL CHECKS PASSED");
    return failed ? 1 : 0;
}
