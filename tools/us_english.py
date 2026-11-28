#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""us_english.py: find, or fix, British spellings in docs prose.

Usage:
    python3 us_english.py PATH...                 # check: list hits, exit 1 if any
    python3 us_english.py --fix PATH...           # rewrite the files in place
    python3 us_english.py --allow FILE PATH...    # skip allowlisted (path, word) pairs

PATH may be a file or a folder (folders are walked for .md .mdx .markdown .txt .ipynb).
Only prose is touched: fenced code blocks, inline `code`, URLs, link targets, HTML tags,
MDX import/export lines and notebook code cells are skipped. The word list below is the
whole rule: no network, no guessing, so the same input always gives the same output (run
it twice: the second --fix changes nothing).

Allowlist file: one entry per line, "<path glob><TAB><word><TAB><reason>", for verbatim
third-party or legal text, proper nouns and quoted product UI; the word * skips the whole
file (for a file generated from code). Lines starting with # are comments.

Keep every copy of this file identical apart from its license header lines: change the
word list in one copy, then copy the file to the others.
"""

import fnmatch
import json
import os
import re
import sys

# -ise/-yse verbs and their family (organise, organised, organisation, organiser...).
ISE = """organ real recogn optim minim maxim priorit summar custom initial serial deserial
normal final util visual parallel vector token synchron standard categor author apolog
emphas special general stabil material memor central local capital character critic
harmon hypothes item mobil modern monet neutral penal personal rational sanit scrutin
symbol theor trivial digit container quant random regular discret linear amort factor
global jeopard public revolution energ familiar econom memo virtual canonical synthes
raster international patron colon fertil immun legal ideal"""
ISE_SUFFIX = "e|ed|es|ing|ation|ations|er|ers|able"
YSE = "anal catal paral"  # analyse -> analyze ("analyses" is left alone: also the US plural)
OUR = "col behavi fav hon lab neighb flav hum rum vap arm endeav rig vig od harb sav"
OUR_SUFFIX = "s|ed|ing|ful|fully|less|able|ably|ite|ites|al|ally|ist|ists|er|ers"
RE_WORDS = "cent met lit fib theat calib somb spect meag"  # centre -> center; not "lust" (Lustre)
PREFIXES = "un|re|de|non|pre|multi|over|under|mis|auto|co|sub|inter|hyper|micro"
WORDS = {
    "catalogue": "catalog",
    "catalogues": "catalogs",
    "catalogued": "cataloged",
    "analogue": "analog",
    "analogues": "analogs",
    "licence": "license",
    "licences": "licenses",
    "defence": "defense",
    "offence": "offense",
    "pretence": "pretense",
    "grey": "gray",
    "greys": "grays",
    "greyed": "grayed",
    "greyscale": "grayscale",
    "programme": "program",
    "programmes": "programs",
    "whilst": "while",
    "amongst": "among",
    "artefact": "artifact",
    "artefacts": "artifacts",
    "judgement": "judgment",
    "judgements": "judgments",
    "acknowledgement": "acknowledgment",
    "acknowledgements": "acknowledgments",
    "ageing": "aging",
    "sceptical": "skeptical",
    "sceptic": "skeptic",
    "aluminium": "aluminum",
    "learnt": "learned",
    "spelt": "spelled",
    "speciality": "specialty",
    "maths": "math",
    "fulfil": "fulfill",
    "fulfils": "fulfills",
    "fulfilment": "fulfillment",
    "enrol": "enroll",
    "enrols": "enrolls",
    "enrolment": "enrollment",
    "skilful": "skillful",
    "wilful": "willful",
    "focussed": "focused",
    "focussing": "focusing",
    "benefitted": "benefited",
    "benefitting": "benefiting",
    "modelling": "modeling",
    "modelled": "modeled",
    "modeller": "modeler",
    "labelled": "labeled",
    "labelling": "labeling",
    "travelled": "traveled",
    "travelling": "traveling",
    "cancelled": "canceled",
    "cancelling": "canceling",
    "signalling": "signaling",
    "signalled": "signaled",
    "levelled": "leveled",
    "levelling": "leveling",
    "fuelled": "fueled",
    "channelled": "channeled",
    "tunnelling": "tunneling",
    "marshalling": "marshaling",
    "marshalled": "marshaled",
    "totalled": "totaled",
    "totalling": "totaling",
    "counsellor": "counselor",
    "jewellery": "jewelry",
    "manoeuvre": "maneuver",
    "mould": "mold",
    "per cent": "percent",
}

_PRE = f"({PREFIXES})?"
_PATTERN_SOURCES = [
    (rf"\b{_PRE}({'|'.join(ISE.split())})is({ISE_SUFFIX})\b", lambda m: f"{m[2]}iz{m[3]}"),
    (rf"\b{_PRE}({'|'.join(YSE.split())})ys(e|ed|ing|er|ers)\b", lambda m: f"{m[2]}yz{m[3]}"),
    (rf"\b{_PRE}({'|'.join(OUR.split())})our({OUR_SUFFIX})?\b", lambda m: f"{m[2]}or{m[3] or ''}"),
    (
        rf"\b{_PRE}({'|'.join(RE_WORDS.split())})re(s|d)?\b",
        lambda m: f"{m[2]}er{'ed' if m[3] == 'd' else m[3] or ''}",
    ),
]
PATTERNS = [(re.compile(p, re.IGNORECASE), f) for p, f in _PATTERN_SOURCES]
_LONGEST_FIRST = sorted(map(re.escape, WORDS), key=len, reverse=True)
WORD_RE = re.compile(rf"\b{_PRE}({'|'.join(_LONGEST_FIRST)})\b", re.IGNORECASE)
# Inline code, URLs, link targets and HTML tags are never prose.
SKIP_INLINE = re.compile(r"`[^`\n]*`|https?://\S+|\]\([^)\s]*\)|<[^<>\n]+>")


def _case(src, dst):
    """dst in the case of src: UPPER, Capitalized or lower."""
    if src.isupper():
        return dst.upper()
    if src[:1].isupper():
        return dst[:1].upper() + dst[1:]
    return dst


def _fix_text(text, skip=()):
    """Return (new_text, [(word, american)]) for one line of prose; words in skip are kept."""
    hits = []

    def replacer(american):
        def sub(m):
            if m[0].lower() in skip:
                return m[0]
            new = _case(m[0], ((m[1] or "") + american(m)).lower())
            hits.append((m[0], new))
            return new

        return sub

    def prose(s):
        s = WORD_RE.sub(replacer(lambda m: WORDS[m[2].lower()]), s)
        for rx, american in PATTERNS:
            s = rx.sub(replacer(american), s)
        return s

    out, last = [], 0
    for m in SKIP_INLINE.finditer(text):  # rewrite only the parts between skipped spans
        out += [prose(text[last : m.start()]), m[0]]
        last = m.end()
    out.append(prose(text[last:]))
    return "".join(out), hits


def fix_markdown(text, skip=()):
    """Fix a Markdown/MDX/text document, skipping fenced code and MDX import/export lines."""
    lines, hits, fence = text.splitlines(keepends=True), [], None
    for i, line in enumerate(lines):
        s = line.lstrip()
        if fence:
            if s.startswith(fence):
                fence = None
            continue
        if s.startswith(("```", "~~~")):
            fence = s[:3]
            continue
        if re.match(r"(import|export)\s", s):
            continue
        lines[i], found = _fix_text(line, skip)
        hits += [(i + 1, old, new) for old, new in found]
    return "".join(lines), hits


def fix_notebook(text, skip=()):
    """Fix the Markdown cells of a notebook; the file is rewritten only when a cell changed."""
    nb = json.loads(text)
    hits = []
    for ci, cell in enumerate(nb.get("cells", [])):
        if cell.get("cell_type") != "markdown":
            continue
        source = cell["source"]
        new, found = fix_markdown(source if isinstance(source, str) else "".join(source), skip)
        if found:
            cell["source"] = new if isinstance(source, str) else new.splitlines(keepends=True)
            hits += [(f"cell {ci} line {ln}", old, us) for ln, old, us in found]
    out = json.dumps(nb, indent=1, ensure_ascii=False) + "\n" if hits else text
    return out, hits


EXTS = (".md", ".mdx", ".markdown", ".txt", ".ipynb")
SKIP_DIRS = {
    ".git",
    ".claude",
    "node_modules",
    "dist",
    "build",
    ".astro",
    ".docusaurus",
    "__pycache__",
    ".venv",
}


def walk(paths):
    for p in paths:
        if os.path.isfile(p):
            yield p
            continue
        for root, dirs, files in os.walk(p):
            dirs[:] = sorted(d for d in dirs if d not in SKIP_DIRS)
            for f in sorted(files):
                if f.endswith(EXTS):
                    yield os.path.join(root, f)


def load_allow(path):
    allow = []
    with open(path, encoding="utf-8") as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                glob, word = line.rstrip("\n").split("\t")[:2]
                allow.append((glob, word.lower()))
    return allow


def main(argv):
    fix = "--fix" in argv
    args = [a for a in argv if a != "--fix"]
    allow = []
    if "--allow" in args:
        i = args.index("--allow")
        allow = load_allow(args[i + 1])
        del args[i : i + 2]
    if not args:
        print(__doc__)
        return 2
    total = 0
    for path in walk(args):
        with open(path, encoding="utf-8") as f:
            text = f.read()
        skip = {w for g, w in allow if fnmatch.fnmatch(path, g)}
        if "*" in skip:
            continue
        fixer = fix_notebook if path.endswith(".ipynb") else fix_markdown
        new, hits = fixer(text, skip)
        total += len(hits)
        for where, old, us in hits:
            print(f"{path}:{where}: {old} -> {us}")
        if fix and new != text:
            with open(path, "w", encoding="utf-8") as f:
                f.write(new)
    if total and not fix:
        name = os.path.basename(__file__)
        print(f"{total} British spelling(s). Fix: python3 {name} --fix <paths>", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
