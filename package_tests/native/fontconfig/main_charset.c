/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for fontconfig: charset; calls FcCharSetCreate(), FcCharSetAddChar(), FcCharSetCount(), FcCharSetHasChar().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <fontconfig/fontconfig.h>
#include <assert.h>
#include <stdio.h>

int main(void) {
    FcCharSet *a = FcCharSetCreate();
    assert(a != NULL);
    assert(FcCharSetAddChar(a, (FcChar32)'A') == FcTrue);
    assert(FcCharSetAddChar(a, (FcChar32)'B') == FcTrue);
    assert(FcCharSetAddChar(a, (FcChar32)'C') == FcTrue);

    FcChar32 count_a = FcCharSetCount(a);
    assert(count_a == 3);
    assert(FcCharSetHasChar(a, (FcChar32)'A') == FcTrue);
    assert(FcCharSetHasChar(a, (FcChar32)'B') == FcTrue);
    assert(FcCharSetHasChar(a, (FcChar32)'Z') == FcFalse);

    FcCharSet *b = FcCharSetCreate();
    assert(b != NULL);
    assert(FcCharSetAddChar(b, (FcChar32)'B') == FcTrue);
    assert(FcCharSetAddChar(b, (FcChar32)'C') == FcTrue);
    assert(FcCharSetAddChar(b, (FcChar32)'D') == FcTrue);

    /* Intersection cardinality |A ∩ B| must be 2 ({B, C}). */
    FcChar32 inter = FcCharSetIntersectCount(a, b);
    assert(inter == 2);

    /* Neither A nor B is a subset of the other (each has a unique element). */
    assert(FcCharSetIsSubset(a, b) == FcFalse);
    assert(FcCharSetIsSubset(b, a) == FcFalse);

    /* But A is a subset of itself — reflexivity check. */
    assert(FcCharSetIsSubset(a, a) == FcTrue);

    /* Copy semantics: copy of A equals A (same count, same membership). */
    FcCharSet *a_copy = FcCharSetCopy(a);
    assert(a_copy != NULL);
    assert(FcCharSetCount(a_copy) == count_a);
    assert(FcCharSetEqual(a, a_copy) == FcTrue);

    FcCharSetDestroy(a_copy);
    FcCharSetDestroy(b);
    FcCharSetDestroy(a);

    printf("fontconfig charset test OK: |A|=%u, |A∩B|=%u\n",
           (unsigned)count_a, (unsigned)inter);
    return 0;
}
