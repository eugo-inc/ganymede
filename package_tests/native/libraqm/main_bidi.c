/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libraqm: bidi; calls FT_New_Memory_Face(), FT_Set_Char_Size(), raqm_create(), raqm_set_text().
 */

#include <raqm.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* A minimal, structurally-valid TrueType font (.notdef + space glyphs only),
 * base64-encoded. Enough for FreeType to instantiate a face so raqm_layout()
 * runs the fribidi BiDi pass. Glyph coverage is irrelevant to direction
 * resolution. */
static const char MIN_TTF_B64[] =
    "AAEAAAAKAIAAAwAgT1MvMkEXQVgAAAEoAAAAYGNtYXAADABzAAABkAAAADRnbHlmAAAAAAAAAcwAAAAB"
    "aGVhZCu1AR4AAACsAAAANmhoZWEDIgEuAAAA5AAAACRobXR4AfQAAAAAAYgAAAAGbG9jYQAAAAAAAAHE"
    "AAAABm1heHAAAwACAAABCAAAACBuYW1lTj9k4wAAAdAAAABgcG9zdAAHAAAAAAIwAAAAJgABAAAAAQAA"
    "JSlLKl8PPPUAAwPoAAAAAOZPYB8AAAAA5k9gHwAAAAAAAAAAAAAAAwACAAAAAAAAAAEAAAMg/zgAAAH0"
    "AAAAAAAAAAEAAAAAAAAAAAAAAAAAAAABAAEAAAACAAAAAAAAAAAAAgAAAAAAAAAAAAAAAAAAAAAAAwH0"
    "AZAABQAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAPz8/PwAA"
    "ACAAIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgAAAB9AAAAAAAAAAAAAIAAAADAAAAFAADAAEAAAAU"
    "AAQAIAAAAAQABAABAAAAIP//AAAAIP///+EAAQAAAAAAAAAAAAAAAAAAAAAAAAAEADYAAQAAAAAAAQAH"
    "AAAAAQAAAAAAAgAHAAcAAwABBAkAAQAOAA4AAwABBAkAAgAOABxFdWdvTWluUmVndWxhcgBFAHUAZwBv"
    "AE0AaQBuAFIAZQBnAHUAbABhAHIAAgAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAwAA";

/* Minimal RFC 4648 base64 decoder. Returns decoded length, or -1 on error. */
static long b64_decode(const char *in, unsigned char *out, size_t out_cap) {
    static const signed char T[256] = {
        ['A'] = 0,  ['B'] = 1,  ['C'] = 2,  ['D'] = 3,  ['E'] = 4,  ['F'] = 5,
        ['G'] = 6,  ['H'] = 7,  ['I'] = 8,  ['J'] = 9,  ['K'] = 10, ['L'] = 11,
        ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
        ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
        ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
        ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
        ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
        ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
        ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
        ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
        ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63,
    };
    size_t out_len = 0;
    uint32_t acc = 0;
    int bits = 0;
    for (const char *p = in; *p; ++p) {
        if (*p == '=' || *p == '\n' || *p == '\r')
            continue;
        signed char v = T[(unsigned char)*p];
        if (v < 0 && *p != 'A')
            return -1;
        acc = (acc << 6) | (uint32_t)v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (out_len >= out_cap)
                return -1;
            out[out_len++] = (unsigned char)((acc >> bits) & 0xFF);
        }
    }
    return (long)out_len;
}

static FT_Library g_lib;
static unsigned char g_font[2048];
static long g_font_len;

static raqm_direction_t resolve_direction(const uint32_t *codepoints, size_t len) {
    FT_Face face = NULL;
    int rc = FT_New_Memory_Face(g_lib, g_font, g_font_len, 0, &face);
    assert(rc == 0 && face != NULL);
    assert(FT_Set_Char_Size(face, 0, 12 * 64, 72, 72) == 0);

    raqm_t *rq = raqm_create();
    assert(rq != NULL);
    assert(raqm_set_text(rq, codepoints, len) == 1);
    assert(raqm_set_freetype_face(rq, face) == 1);
    assert(raqm_set_par_direction(rq, RAQM_DIRECTION_DEFAULT) == 1);
    /* The fribidi BiDi pass (and thus direction resolution) runs here. */
    assert(raqm_layout(rq) == 1);
    raqm_direction_t resolved = raqm_get_par_resolved_direction(rq);

    raqm_destroy(rq);
    FT_Done_Face(face);
    return resolved;
}

int main(void) {
    assert(FT_Init_FreeType(&g_lib) == 0);
    g_font_len = b64_decode(MIN_TTF_B64, g_font, sizeof(g_font));
    assert(g_font_len > 0);

    /* Pure Latin "Hello" -- must resolve to LTR. */
    const uint32_t latin[] = { 'H', 'e', 'l', 'l', 'o' };
    raqm_direction_t ltr = resolve_direction(latin, sizeof(latin) / sizeof(latin[0]));
    assert(ltr == RAQM_DIRECTION_LTR);

    /* Hebrew "שלום" (shin-lamed-vav-mem) -- must resolve to RTL. */
    const uint32_t hebrew[] = { 0x05E9, 0x05DC, 0x05D5, 0x05DD };
    raqm_direction_t rtl_he = resolve_direction(hebrew, sizeof(hebrew) / sizeof(hebrew[0]));
    assert(rtl_he == RAQM_DIRECTION_RTL);

    /* Arabic "سلام" (seen-lam-alif-mem) -- must also resolve to RTL. */
    const uint32_t arabic[] = { 0x0633, 0x0644, 0x0627, 0x0645 };
    raqm_direction_t rtl_ar = resolve_direction(arabic, sizeof(arabic) / sizeof(arabic[0]));
    assert(rtl_ar == RAQM_DIRECTION_RTL);

    FT_Done_FreeType(g_lib);
    printf("libraqm BiDi resolution test OK\n");
    return 0;
}
