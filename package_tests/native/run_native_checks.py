#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo, Inc.
"""Install checks for native libraries and tools under one prefix.

Each package folder next to this script holds a ``testspec.json`` (see SCHEMA.md) and the
small C/C++ programs it names. For every selected spec the runner checks, in order:

1. artifacts  - every listed file exists under the prefix (``lib`` and ``lib64`` are
                interchangeable, a shared library may carry a different version suffix,
                glob rows must match something, and a dangling symlink is a failure);
2. needed     - every NEEDED entry in the dynamic section of each ELF artifact resolves
                to a compatible library (RPATH/RUNPATH, LD_LIBRARY_PATH, the loader
                cache and configuration, the default system directories, or the
                prefix's own lib directories);
3. pic        - when the spec asks for it, every static archive links into a shared
                object (when the linker cannot say, a relocation scan can still fail an
                archive, but never passes one);
4. smoke      - an optional command from the prefix runs, exits 0 and prints the
                expected text;
5. programs   - each test program compiles, links against the prefix and runs.

A spec PASSes when no check failed and at least one ran, is SKIPped when every check was
skipped, and FAILs otherwise. The exit status is 1 when any spec failed.

Only the Python standard library is used. Linux (ELF) only.
"""
from __future__ import annotations

import argparse
import fnmatch
import glob
import json
import os
import platform
import re
import shlex
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor
from dataclasses import asdict, dataclass, field
from pathlib import Path

HERE = Path(__file__).resolve().parent

KINDS = {
    "shared-libs-only",
    "shared-and-static-libs-with-cli",
    "shared-and-static-libs",
    "shared-libs-with-cli",
    "static-libs-only",
    "header-only",
    "cli-tool-only",
}
CAPABILITIES = {"gpu", "cuda-driver", "toolchain", "rdma", "nvidia-driver"}
TOP_KEYS = {"name", "kind", "expected_artifacts", "expected_artifact_globs", "static_archives_must_be_pic",
            "smoke_run", "test_programs", "requires"}
SMOKE_KEYS = {"command", "args", "expect_stdout_contains", "timeout_s", "requires"}
PROGRAM_KEYS = {"source", "language", "link_with", "extra_cflags", "runtime_timeout_s", "compile_timeout_s", "requires"}

DEFAULT_SMOKE_TIMEOUT_S = 30.0
DEFAULT_RUNTIME_TIMEOUT_S = 10.0
DEFAULT_COMPILE_TIMEOUT_S = 30.0
DETAIL_LIMIT = 600

PASS, FAIL, SKIP = "PASS", "FAIL", "SKIP"


# --------------------------------------------------------------------------------------------
# results


@dataclass
class Check:
    stage: str  # artifacts | needed | pic | smoke | programs | spec
    target: str
    status: str  # PASS | FAIL | SKIP
    detail: str = ""


@dataclass
class SpecResult:
    package: str
    result: str = SKIP
    checks: list = field(default_factory=list)
    seconds: float = 0.0

    def add(self, stage, target, status, detail=""):
        self.checks.append(Check(stage, target, status, clip(detail)))

    def finish(self):
        statuses = [c.status for c in self.checks]
        if FAIL in statuses:
            self.result = FAIL
        elif PASS in statuses:
            self.result = PASS
        else:
            self.result = SKIP

    def counts(self, stage):
        rows = [c for c in self.checks if c.stage == stage]
        if not rows:
            return "-"
        ok = sum(c.status == PASS for c in rows)
        ran = sum(c.status != SKIP for c in rows)
        return f"{ok}/{ran}" if ran else "skip"


def clip(text, limit=DETAIL_LIMIT):
    text = (text or "").strip()
    return text if len(text) <= limit else text[: limit - 3] + "..."


# --------------------------------------------------------------------------------------------
# spec loading


class SpecError(Exception):
    pass


def _expect(cond, message):
    if not cond:
        raise SpecError(message)


def _str_list(value, where):
    _expect(isinstance(value, list) and all(isinstance(v, str) for v in value), f"{where} must be a list of strings")
    return value


def _timeout(value, where, ceiling):
    _expect(isinstance(value, (int, float)) and not isinstance(value, bool), f"{where} must be a number")
    _expect(0 < float(value) <= ceiling, f"{where} must be above 0 and at most {ceiling}")
    return float(value)


def _requires(value, where):
    if value is None:
        return None
    _expect(value in CAPABILITIES, f"{where}: unknown capability {value!r} (known: {', '.join(sorted(CAPABILITIES))})")
    return value


def load_spec(path: Path) -> dict:
    try:
        spec = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        raise SpecError(f"cannot read {path.name}: {error}") from None
    _expect(isinstance(spec, dict), "the spec must be a JSON object")
    unknown = set(spec) - TOP_KEYS
    _expect(not unknown, f"unknown key(s): {', '.join(sorted(unknown))}")
    _expect(isinstance(spec.get("name"), str) and spec["name"], "name is required")
    _expect(spec.get("kind") in KINDS, f"kind must be one of: {', '.join(sorted(KINDS))}")
    spec.setdefault("expected_artifacts", [])
    spec.setdefault("expected_artifact_globs", [])
    _str_list(spec["expected_artifacts"], "expected_artifacts")
    _str_list(spec["expected_artifact_globs"], "expected_artifact_globs")
    spec.setdefault("static_archives_must_be_pic", False)
    _expect(isinstance(spec["static_archives_must_be_pic"], bool), "static_archives_must_be_pic must be true or false")
    spec["requires"] = _requires(spec.get("requires"), "requires")

    smoke = spec.get("smoke_run")
    if smoke is not None:
        _expect(isinstance(smoke, dict), "smoke_run must be an object or null")
        unknown = set(smoke) - SMOKE_KEYS
        _expect(not unknown, f"smoke_run: unknown key(s): {', '.join(sorted(unknown))}")
        _expect(isinstance(smoke.get("command"), str) and smoke["command"], "smoke_run.command is required")
        smoke["args"] = _str_list(smoke.get("args", []), "smoke_run.args")
        want = smoke.get("expect_stdout_contains")
        _expect(want is None or isinstance(want, str), "smoke_run.expect_stdout_contains must be a string or null")
        smoke["timeout_s"] = _timeout(smoke.get("timeout_s", DEFAULT_SMOKE_TIMEOUT_S), "smoke_run.timeout_s", 600)
        smoke["requires"] = _requires(smoke.get("requires"), "smoke_run.requires")
    spec["smoke_run"] = smoke

    programs = spec.get("test_programs", [])
    _expect(isinstance(programs, list), "test_programs must be a list")
    for index, prog in enumerate(programs):
        where = f"test_programs[{index}]"
        _expect(isinstance(prog, dict), f"{where} must be an object")
        unknown = set(prog) - PROGRAM_KEYS
        _expect(not unknown, f"{where}: unknown key(s): {', '.join(sorted(unknown))}")
        _expect(isinstance(prog.get("source"), str) and prog["source"], f"{where}.source is required")
        _expect("/" not in prog["source"] and ".." not in prog["source"], f"{where}.source must be a file name")
        prog.setdefault("language", "c")
        _expect(prog["language"] in ("c", "cpp"), f"{where}.language must be 'c' or 'cpp'")
        prog["link_with"] = _str_list(prog.get("link_with", []), f"{where}.link_with")
        prog["extra_cflags"] = _str_list(prog.get("extra_cflags", []), f"{where}.extra_cflags")
        prog["runtime_timeout_s"] = _timeout(prog.get("runtime_timeout_s", DEFAULT_RUNTIME_TIMEOUT_S),
                                             f"{where}.runtime_timeout_s", 300)
        prog["compile_timeout_s"] = _timeout(prog.get("compile_timeout_s", DEFAULT_COMPILE_TIMEOUT_S),
                                             f"{where}.compile_timeout_s", 300)
        prog["requires"] = _requires(prog.get("requires"), f"{where}.requires")
    spec["test_programs"] = programs
    return spec


def discover(specs_dir: Path, patterns):
    found = sorted(p.parent for p in specs_dir.glob("*/testspec.json"))
    if not patterns:
        return found
    chosen = []
    for folder in found:
        if any(fnmatch.fnmatch(folder.name, pat) for pat in patterns):
            chosen.append(folder)
    return chosen


# --------------------------------------------------------------------------------------------
# ELF helpers (dynamic section only)

PT_LOAD, PT_DYNAMIC = 1, 2
TAG_NEEDED, TAG_STRTAB, TAG_STRSZ, TAG_RPATH, TAG_RUNPATH = 1, 5, 10, 15, 29


@dataclass
class ElfInfo:
    elf_class: int
    machine: int
    elf_type: int
    needed: list
    rpath: list
    runpath: list


def elf_identity(path):
    """(class, machine, type) for an ELF file, or None."""
    try:
        with open(path, "rb") as handle:
            head = handle.read(20)
    except OSError:
        return None
    if len(head) < 20 or head[:4] != b"\x7fELF":
        return None
    order = "<" if head[5] == 1 else ">"
    elf_type, machine = struct.unpack(order + "HH", head[16:20])
    return head[4], machine, elf_type


def read_elf(path) -> ElfInfo | None:
    try:
        with open(path, "rb") as handle:
            ident = handle.read(64)
            if len(ident) < 52 or ident[:4] != b"\x7fELF":
                return None
            is64 = ident[4] == 2
            order = "<" if ident[5] == 1 else ">"
            elf_type, machine = struct.unpack(order + "HH", ident[16:20])
            if is64:
                (phoff,) = struct.unpack(order + "Q", ident[32:40])
                phentsize, phnum = struct.unpack(order + "HH", ident[54:58])
                ph_fmt, dyn_fmt = order + "IIQQQQQQ", order + "qQ"
            else:
                (phoff,) = struct.unpack(order + "I", ident[28:32])
                phentsize, phnum = struct.unpack(order + "HH", ident[42:46])
                ph_fmt, dyn_fmt = order + "IIIIIIII", order + "iI"
            loads, dynamic = [], None
            for index in range(phnum):
                handle.seek(phoff + index * phentsize)
                raw = handle.read(struct.calcsize(ph_fmt))
                if len(raw) < struct.calcsize(ph_fmt):
                    break
                fields = struct.unpack(ph_fmt, raw)
                if is64:
                    p_type, _flags, p_offset, p_vaddr, _paddr, p_filesz = fields[:6]
                else:
                    p_type, p_offset, p_vaddr, _paddr, p_filesz = fields[:5]
                if p_type == PT_LOAD:
                    loads.append((p_vaddr, p_offset, p_filesz))
                elif p_type == PT_DYNAMIC:
                    dynamic = (p_offset, p_filesz)
            info = ElfInfo(ident[4], machine, elf_type, [], [], [])
            if dynamic is None:
                return info
            handle.seek(dynamic[0])
            blob = handle.read(dynamic[1])
            size = struct.calcsize(dyn_fmt)
            entries = []
            for start in range(0, len(blob) - size + 1, size):
                tag, value = struct.unpack(dyn_fmt, blob[start:start + size])
                if tag == 0:
                    break
                entries.append((tag, value))
            strtab = next((v for t, v in entries if t == TAG_STRTAB), None)
            strsz = next((v for t, v in entries if t == TAG_STRSZ), None)
            if strtab is None or not strsz:
                return info
            offset = None
            for vaddr, foff, filesz in loads:
                if vaddr <= strtab < vaddr + filesz:
                    offset = foff + (strtab - vaddr)
                    break
            if offset is None:
                offset = strtab  # prelinked or unusual layout: the address is the file offset
            handle.seek(offset)
            table = handle.read(strsz)

            def text_at(index):
                end = table.find(b"\0", index)
                return table[index:end if end >= 0 else len(table)].decode("utf-8", "replace")

            for tag, value in entries:
                if tag == TAG_NEEDED:
                    info.needed.append(text_at(value))
                elif tag == TAG_RPATH:
                    info.rpath += [p for p in text_at(value).split(":") if p]
                elif tag == TAG_RUNPATH:
                    info.runpath += [p for p in text_at(value).split(":") if p]
            return info
    except (OSError, struct.error):
        return None


def read_elf_with_readelf(path) -> ElfInfo | None:
    """Fallback when the built-in reader cannot parse a file: ask readelf -d."""
    tool = shutil.which("readelf") or shutil.which("llvm-readelf")
    ident = elf_identity(path)
    if not tool or ident is None:
        return None
    try:
        out = subprocess.run([tool, "-d", str(path)], capture_output=True, text=True, errors="replace",
                             timeout=60).stdout
    except (OSError, subprocess.SubprocessError):
        return None
    info = ElfInfo(ident[0], ident[1], ident[2], [], [], [])
    for line in out.splitlines():
        found = re.search(r"\((NEEDED|RPATH|RUNPATH)\)\s+.*?\[(.*)\]", line)
        if not found:
            continue
        kind, value = found.groups()
        if kind == "NEEDED":
            info.needed.append(value)
        elif kind == "RPATH":
            info.rpath += [p for p in value.split(":") if p]
        else:
            info.runpath += [p for p in value.split(":") if p]
    return info


class LibrarySearch:
    """Where the dynamic loader would look, plus the prefix's own lib directories."""

    def __init__(self, prefix: Path):
        self.prefix_dirs = prefix_lib_dirs(prefix)
        self.system_dirs = self._system_dirs()
        self.cache = self._loader_cache()
        self.env_dirs = [Path(p) for p in os.environ.get("LD_LIBRARY_PATH", "").split(":") if p]
        self._ident_cache = {}

    @staticmethod
    def _conf_dirs(conf: Path, seen):
        dirs = []
        if conf in seen or not conf.is_file():
            return dirs
        seen.add(conf)
        try:
            lines = conf.read_text(errors="replace").splitlines()
        except OSError:
            return dirs
        for line in lines:
            line = line.split("#", 1)[0].strip()
            if not line:
                continue
            if line.startswith("include "):
                pattern = line.split(None, 1)[1]
                if not pattern.startswith("/"):
                    pattern = str(conf.parent / pattern)
                for match in sorted(glob.glob(pattern)):
                    dirs += LibrarySearch._conf_dirs(Path(match), seen)
            elif line.startswith("/"):
                dirs.append(Path(line))
        return dirs

    def _system_dirs(self):
        dirs = self._conf_dirs(Path("/etc/ld.so.conf"), set())
        for base in ("/lib", "/lib64", "/usr/lib", "/usr/lib64"):
            dirs.append(Path(base))
            dirs += [Path(base) / t for t in TRIPLETS]
        unique = []
        for d in dirs:
            if d not in unique:
                unique.append(d)
        return unique

    @staticmethod
    def _loader_cache():
        tool = shutil.which("ldconfig") or ("/sbin/ldconfig" if os.path.exists("/sbin/ldconfig") else None)
        cache = {}
        if not tool:
            return cache
        try:
            out = subprocess.run([tool, "-p"], capture_output=True, text=True, errors="replace", timeout=30).stdout
        except (OSError, subprocess.SubprocessError):
            return cache
        for line in out.splitlines():
            found = re.match(r"\s*(\S+)\s+\(.*\)\s+=>\s+(\S+)", line)
            if found:
                cache.setdefault(found.group(1), []).append(Path(found.group(2)))
        return cache

    def _compatible(self, candidate: Path, want):
        key = str(candidate)
        if key not in self._ident_cache:
            self._ident_cache[key] = elf_identity(candidate) if candidate.exists() else None
        ident = self._ident_cache[key]
        return ident is not None and ident[0] == want[0] and ident[1] == want[1]

    @staticmethod
    def _expand(entry: str, origin: Path, is64: bool):
        entry = entry.replace("${ORIGIN}", str(origin)).replace("$ORIGIN", str(origin))
        entry = entry.replace("${LIB}", "lib64" if is64 else "lib").replace("$LIB", "lib64" if is64 else "lib")
        return Path(entry)

    def resolve(self, name: str, info: ElfInfo, artifact: Path):
        """Return (where, via) for a NEEDED name, or (None, None)."""
        want = (info.elf_class, info.machine)
        if "/" in name:
            path = Path(name) if name.startswith("/") else artifact.parent / name
            return (path, "path") if self._compatible(path, want) else (None, None)
        origin = artifact.resolve().parent
        is64 = info.elf_class == 2
        ordered = []
        if not info.runpath:
            ordered += [("rpath", self._expand(p, origin, is64)) for p in info.rpath]
        ordered += [("LD_LIBRARY_PATH", d) for d in self.env_dirs]
        ordered += [("runpath", self._expand(p, origin, is64)) for p in info.runpath]
        for directory_kind, directory in ordered:
            if self._compatible(directory / name, want):
                return directory / name, directory_kind
        for cached in self.cache.get(name, []):
            if self._compatible(cached, want):
                return cached, "loader cache"
        for directory in self.system_dirs:
            if self._compatible(directory / name, want):
                return directory / name, "system"
        for directory in self.prefix_dirs:
            if self._compatible(directory / name, want):
                return directory / name, "prefix lib dir"
        return None, None


# --------------------------------------------------------------------------------------------
# artifact resolution

VERSIONED_SO = re.compile(r"^(?P<stem>.+?\.so)(?P<ver>(?:\.\d+)+)$")


def multiarch_triplets():
    machine = platform.machine()
    names = [f"{machine}-linux-gnu", f"{machine}-linux-musl"]
    if machine == "armv7l":
        names.append("arm-linux-gnueabihf")
    if machine in ("i386", "i686"):
        names.append("i386-linux-gnu")
    return names


TRIPLETS = multiarch_triplets()


def lib_variants(rel: str):
    """The row itself, then lib <-> lib64, then lib/<multiarch triplet> (Debian-style layouts)."""
    parts = rel.split("/")
    variants = [rel]
    for index, part in enumerate(parts[:-1]):
        if part in ("lib", "lib64"):
            swapped = parts.copy()
            swapped[index] = "lib64" if part == "lib" else "lib"
            variants.append("/".join(swapped))
            for triplet in TRIPLETS:
                variants.append("/".join(parts[:index] + ["lib", triplet] + parts[index + 1:]))
            break
    return variants


def prefix_lib_dirs(prefix: Path):
    dirs = [prefix / "lib", prefix / "lib64"] + [prefix / "lib" / t for t in TRIPLETS]
    return [d for d in dirs if d.is_dir()]


def resolve_artifact(prefix: Path, rel: str):
    """Return (status, path_or_None, note)."""
    for variant in lib_variants(rel):
        path = prefix / variant
        if os.path.lexists(path):
            if not path.exists():
                return FAIL, path, f"dangling symlink {variant} -> {os.readlink(path)}"
            note = "" if variant == rel else f"found as {variant}"
            return PASS, path, note
    name = rel.rsplit("/", 1)[-1]
    versioned = VERSIONED_SO.match(name)
    if versioned:
        stem, version = versioned.group("stem"), versioned.group("ver")
        major = version.split(".")[1]
        for variant in lib_variants(rel):
            folder = (prefix / variant).parent
            if not folder.is_dir():
                continue
            same_major = sorted(folder.glob(f"{glob.escape(stem)}.{major}.*")) + sorted(folder.glob(f"{glob.escape(stem)}.{major}"))
            any_version = sorted(folder.glob(f"{glob.escape(stem)}.*"))
            for pool, label in ((same_major, "same major version"), (any_version, "different version")):
                live = [p for p in pool if p.exists()]
                if live:
                    return PASS, live[0], f"found {live[0].relative_to(prefix)} ({label})"
    return FAIL, None, "missing"


def resolve_glob(prefix: Path, pattern: str):
    matches = []
    for variant in lib_variants(pattern):
        # Escape the prefix so a "[" or "*" in the install path is taken literally.
        matches += [Path(p) for p in glob.glob(os.path.join(glob.escape(str(prefix)), variant), recursive=True)]
    matches = sorted(set(matches))
    if not matches:
        return FAIL, [], "no match"
    dangling = [m for m in matches if os.path.lexists(m) and not m.exists()]
    live = [m for m in matches if m.exists()]
    if dangling:
        return FAIL, live, "dangling symlink(s): " + ", ".join(str(m.relative_to(prefix)) for m in dangling[:5])
    return PASS, live, f"{len(live)} match(es)"


# --------------------------------------------------------------------------------------------
# runner


@dataclass
class Options:
    prefix: Path
    cuda_home: Path
    cc: str
    cxx: str
    capabilities: set
    extra_cflags: list
    ld_path: bool
    timeout_scale: float
    keep: bool
    verbose: bool


def expand(text: str, opts: Options, spec_dir: Path):
    return (text.replace("{prefix}", str(opts.prefix))
                .replace("{cuda_home}", str(opts.cuda_home))
                .replace("{testspec_dir}", str(spec_dir)))


def expand_flags(flags, opts, spec_dir):
    out = []
    for flag in flags:
        flag = expand(flag, opts, spec_dir)
        found = re.match(r"^(-I|-L|-isystem)(.*[*?[].*)$", flag)
        if found:
            hits = sorted(glob.glob(found.group(2)))
            out += [found.group(1) + h for h in hits] if hits else [flag]
        else:
            out.append(flag)
    return out


def expand_link_names(names, lib_dirs):
    """A link_with entry containing ``*``, ``?`` or ``[`` is a glob over ``lib<entry>.so`` and
    ``lib<entry>.a`` in the prefix's lib directories. It becomes the first matching library name
    in sorted order, the same order the ``-I`` globs in extra_cflags use, so headers and library
    agree when several versions are installed. With no match it stays as written and the link
    step reports it."""
    out = []
    for name in names:
        if not re.search(r"[*?[]", name):
            out.append(name)
            continue
        found = set()
        for folder in lib_dirs:
            for suffix in (".so", ".a"):
                for hit in glob.glob(os.path.join(glob.escape(str(folder)), f"lib{name}{suffix}")):
                    found.add(os.path.basename(hit)[len("lib"):-len(suffix)])
        out.append(sorted(found)[0] if found else name)
    return out


def run_env(opts: Options):
    env = dict(os.environ)
    env["PATH"] = str(opts.prefix / "bin") + os.pathsep + env.get("PATH", "")
    if opts.ld_path:
        extra = [str(d) for d in prefix_lib_dirs(opts.prefix)]
        if env.get("LD_LIBRARY_PATH"):
            extra.append(env["LD_LIBRARY_PATH"])
        env["LD_LIBRARY_PATH"] = os.pathsep.join(extra)
    return env


def tail(text, lines=8):
    rows = [r for r in (text or "").strip().splitlines() if r.strip()]
    return " | ".join(rows[-lines:])


def capability_missing(cap, opts: Options):
    if cap is None or cap in opts.capabilities:
        return None
    if cap == "toolchain" and (shutil.which(opts.cc) or shutil.which(opts.cxx)):
        return None
    return f"needs --with {cap}"


def check_artifacts(spec, opts, result, found):
    for rel in spec["expected_artifacts"]:
        status, path, note = resolve_artifact(opts.prefix, rel)
        result.add("artifacts", rel, status, note)
        if path is not None and status == PASS:
            found.append(path)
    for pattern in spec["expected_artifact_globs"]:
        status, paths, note = resolve_glob(opts.prefix, pattern)
        result.add("artifacts", pattern, status, note)
        found += paths


def check_needed(opts, result, found, search: LibrarySearch):
    seen = set()
    for path in found:
        real = os.path.realpath(path)
        if real in seen or not os.path.isfile(real):
            continue
        seen.add(real)
        ident = elf_identity(real)
        if ident is None or ident[2] not in (2, 3):  # executables and shared objects only
            continue
        info = read_elf(real) or read_elf_with_readelf(real)
        label = str(Path(path).relative_to(opts.prefix)) if str(path).startswith(str(opts.prefix)) else str(path)
        if info is None:
            result.add("needed", label, SKIP, "could not read the dynamic section")
            continue
        if not info.needed:
            result.add("needed", label, PASS, "no NEEDED entries")
            continue
        missing, notes = [], []
        for name in info.needed:
            where, via = search.resolve(name, info, Path(path))
            if where is None:
                missing.append(name)
            elif via == "prefix lib dir":
                notes.append(f"{name} only in the prefix lib dirs (no RPATH/loader entry)")
        if missing:
            result.add("needed", label, FAIL, "unresolved: " + ", ".join(missing))
        else:
            result.add("needed", label, PASS, "; ".join(notes) or f"{len(info.needed)} resolved")


NON_PIC_MESSAGES = re.compile(r"recompile with -fPIC|can ?not be used when making a shared object|"
                              r"requires (?:dynamic|unsupported dynamic) R_|text relocation|relocation R_\S+ against|"
                              r"dangerous relocation",
                              re.I)
ABS_RELOCS = re.compile(r"\bR_(?:X86_64_(?:32S?|64)|386_32|AARCH64_ABS(?:16|32|64)|AARCH64_MOVW_UABS_G\d\w*|"
                        r"ARM_ABS32|PPC64_ADDR(?:16\w*|32|64)|RISCV_(?:32|64|HI20))\b")


def relocation_scan(archive: Path):
    tool = shutil.which("readelf") or shutil.which("llvm-readelf")
    if not tool:
        return SKIP, "no compiler verdict and no readelf for the relocation scan"
    try:
        proc = subprocess.run([tool, "-r", "-W", str(archive)], capture_output=True, text=True, errors="replace",
                              timeout=300, stdin=subprocess.DEVNULL)
    except (OSError, subprocess.SubprocessError) as error:
        return SKIP, f"readelf failed: {error}"
    out = proc.stdout
    if re.search(r"not an ELF file|file format not recognized", proc.stderr, re.I):
        return SKIP, "inconclusive: some archive members are not ELF objects (LTO bitcode?)"
    in_text, hits, member = False, [], ""
    for line in out.splitlines():
        if line.startswith("File: "):
            member = line[6:].strip()
            continue
        section = re.match(r"Relocation section '([^']+)'", line)
        if section:
            in_text = section.group(1).startswith((".rela.text", ".rel.text"))
            continue
        if in_text:
            reloc = ABS_RELOCS.search(line)
            if reloc:
                hits.append(f"{member.rsplit('(', 1)[-1].rstrip(')')}: {reloc.group(0)}")
    if hits:
        return FAIL, "absolute relocations in text (relocation scan): " + ", ".join(sorted(set(hits))[:6])
    # Finding none proves little: ordinary non-PIC code uses PC-relative relocations on AArch64 and
    # often on x86-64 too. Only the link probe can show that an archive is position-independent.
    return SKIP, "inconclusive: the relocation scan found no absolute relocations in text, but only a link probe can confirm PIC"


def check_pic(spec, opts, result, found, workdir: Path):
    if not spec["static_archives_must_be_pic"]:
        return
    archives = []
    for path in found:
        try:
            with open(path, "rb") as handle:
                magic = handle.read(8)
        except OSError:
            continue
        if magic in (b"!<arch>\n", b"!<thin>\n") and path not in archives:
            archives.append(path)
    compiler = shutil.which(opts.cc)
    for archive in archives:
        label = str(archive.relative_to(opts.prefix)) if str(archive).startswith(str(opts.prefix)) else str(archive)
        if compiler:
            out = workdir / ("pic_probe_" + re.sub(r"\W", "_", label) + ".so")
            cmd = [compiler, "-shared", "-nostdlib", "-o", str(out), "-Wl,--whole-archive", str(archive),
                   "-Wl,--no-whole-archive", "-Wl,-z,text", "-Wl,--unresolved-symbols=ignore-all",
                   "-Wl,--allow-multiple-definition"]
            try:
                proc = subprocess.run(cmd, capture_output=True, text=True, errors="replace",
                                      timeout=300 * opts.timeout_scale, stdin=subprocess.DEVNULL, cwd=workdir)
            except subprocess.TimeoutExpired:
                result.add("pic", label, SKIP, "link probe timed out")
                continue
            except OSError as error:
                result.add("pic", label, SKIP, f"could not start the link probe: {error}")
                continue
            if proc.returncode == 0:
                result.add("pic", label, PASS, "links into a shared object")
                continue
            evidence = [line for line in proc.stderr.splitlines() if NON_PIC_MESSAGES.search(line)]
            if evidence:
                result.add("pic", label, FAIL, "not position-independent: " + " | ".join(evidence[:3]))
                continue
            status, note = relocation_scan(archive)
            if status == SKIP:
                note += "; the link probe failed for another reason: " + tail(proc.stderr, 3)
            result.add("pic", label, status, note)
            continue
        status, note = relocation_scan(archive)
        result.add("pic", label, status, note + ("" if compiler else f" (no compiler {opts.cc!r} for the link probe)"))


def check_smoke(spec, opts, result, spec_dir: Path, workdir: Path):
    smoke = spec["smoke_run"]
    if smoke is None:
        return
    label = smoke["command"]
    missing = capability_missing(smoke["requires"], opts)
    if missing:
        result.add("smoke", label, SKIP, missing)
        return
    command = expand(smoke["command"], opts, spec_dir)
    path = Path(command) if os.path.isabs(command) else None
    if path is None:
        for variant in lib_variants(command):
            if (opts.prefix / variant).exists():
                path = opts.prefix / variant
                break
    if path is None or not path.exists():
        result.add("smoke", label, FAIL, "command not found under the prefix")
        return
    argv = [str(path)] + [expand(a, opts, spec_dir) for a in smoke["args"]]
    limit = smoke["timeout_s"] * opts.timeout_scale
    try:
        # Run in the spec's scratch folder so a command that writes files leaves nothing behind.
        proc = subprocess.run(argv, capture_output=True, text=True, errors="replace", timeout=limit,
                              stdin=subprocess.DEVNULL, env=run_env(opts), cwd=workdir)
    except subprocess.TimeoutExpired:
        result.add("smoke", label, FAIL, f"timed out after {limit:g}s")
        return
    except OSError as error:
        result.add("smoke", label, FAIL, f"could not start: {error}")
        return
    if proc.returncode != 0:
        result.add("smoke", label, FAIL, f"exit {proc.returncode}: " + tail(proc.stderr or proc.stdout, 4))
        return
    want = smoke.get("expect_stdout_contains")
    if want is not None and want not in proc.stdout:
        result.add("smoke", label, FAIL, f"stdout lacks {want!r}: " + tail(proc.stdout, 3))
        return
    result.add("smoke", label, PASS, "exit 0" + (f", stdout has {want!r}" if want else ""))


def check_programs(spec, opts, result, spec_dir: Path, workdir: Path):
    lib_dirs = prefix_lib_dirs(opts.prefix)
    for prog in spec["test_programs"]:
        label = prog["source"]
        missing = capability_missing(prog["requires"], opts)
        if missing:
            result.add("programs", label, SKIP, missing)
            continue
        compiler_name = opts.cxx if prog["language"] == "cpp" else opts.cc
        compiler = shutil.which(compiler_name)
        if not compiler:
            result.add("programs", label, SKIP, f"no compiler {compiler_name!r} (set --cc/--cxx or CC/CXX)")
            continue
        source = spec_dir / prog["source"]
        if not source.is_file():
            result.add("programs", label, FAIL, "source file missing beside testspec.json")
            continue
        exe = workdir / (Path(label).stem + "_" + prog["language"])
        cmd = [compiler, *expand_flags(prog["extra_cflags"], opts, spec_dir), *opts.extra_cflags,
               f"-I{opts.prefix / 'include'}", str(source), "-o", str(exe)]
        cmd += [f"-L{d}" for d in lib_dirs] + [f"-Wl,-rpath,{d}" for d in lib_dirs]
        cmd += [f"-l{lib}" for lib in expand_link_names(prog["link_with"], lib_dirs)]
        compile_limit = prog["compile_timeout_s"] * opts.timeout_scale
        started = time.monotonic()
        try:
            proc = subprocess.run(cmd, capture_output=True, text=True, errors="replace", timeout=compile_limit,
                                  cwd=workdir, stdin=subprocess.DEVNULL)
        except subprocess.TimeoutExpired:
            result.add("programs", label, FAIL, f"compile timed out after {compile_limit:g}s")
            continue
        except OSError as error:
            result.add("programs", label, FAIL, f"could not start the compiler: {error}")
            continue
        if proc.returncode != 0:
            stage = "link" if re.search(r"undefined reference|cannot find -l|ld returned|linker command failed",
                                        proc.stderr) else "compile"
            result.add("programs", label, FAIL, f"{stage} failed: " + tail(proc.stderr, 6))
            if opts.verbose:
                result.add("programs", label + " (command)", SKIP, shlex.join(cmd))
            continue
        run_limit = prog["runtime_timeout_s"] * opts.timeout_scale
        try:
            ran = subprocess.run([str(exe)], capture_output=True, text=True, errors="replace", timeout=run_limit,
                                 cwd=workdir, stdin=subprocess.DEVNULL, env=run_env(opts))
        except subprocess.TimeoutExpired:
            result.add("programs", label, FAIL, f"run timed out after {run_limit:g}s")
            continue
        except OSError as error:
            result.add("programs", label, FAIL, f"could not start the built program: {error}")
            continue
        if ran.returncode != 0:
            result.add("programs", label, FAIL, f"run exit {ran.returncode}: " + tail(ran.stderr or ran.stdout, 6))
            continue
        result.add("programs", label, PASS, f"{time.monotonic() - started:.1f}s: " + tail(ran.stdout, 1))


def run_spec(spec_dir: Path, opts: Options, search: LibrarySearch) -> SpecResult:
    result = SpecResult(spec_dir.name)
    started = time.monotonic()
    try:
        spec = load_spec(spec_dir / "testspec.json")
        if spec["name"] != spec_dir.name:
            raise SpecError(f"name {spec['name']!r} does not match its folder {spec_dir.name!r}")
    except SpecError as error:
        result.add("spec", "testspec.json", FAIL, f"invalid spec: {error}")
        result.finish()
        return result
    missing = capability_missing(spec["requires"], opts)
    if missing:
        result.add("spec", spec["name"], SKIP, missing)
        result.finish()
        return result
    workdir = Path(tempfile.mkdtemp(prefix=f"native_check_{spec_dir.name}_"))
    try:
        found = []
        check_artifacts(spec, opts, result, found)
        check_needed(opts, result, found, search)
        check_pic(spec, opts, result, found, workdir)
        check_smoke(spec, opts, result, spec_dir, workdir)
        check_programs(spec, opts, result, spec_dir, workdir)
    except Exception as error:  # one unexpected file or tool must not stop the other specs
        result.add("spec", spec["name"], FAIL, f"runner error: {type(error).__name__}: {error}")
    finally:
        if opts.keep:
            result.add("spec", "workdir", SKIP, f"kept {workdir}")
        else:
            shutil.rmtree(workdir, ignore_errors=True)
    result.finish()
    result.seconds = round(time.monotonic() - started, 2)
    return result


TESTSPEC_DIR_FILE = re.compile(r"\{testspec_dir\}/([^\s'\"]+)")


def spec_problems(spec_dir: Path):
    """Problems that make a spec unusable on any machine: invalid JSON or fields, a name that does
    not match its folder, or a file the spec names that is not beside it."""
    try:
        spec = load_spec(spec_dir / "testspec.json")
    except SpecError as error:
        return [f"invalid spec: {error}"]
    problems = []
    if spec["name"] != spec_dir.name:
        problems.append(f"name {spec['name']!r} does not match its folder {spec_dir.name!r}")
    for prog in spec["test_programs"]:
        if not (spec_dir / prog["source"]).is_file():
            problems.append(f"test program source {prog['source']!r} is missing")
    smoke = spec["smoke_run"]
    if smoke is not None:
        for arg in [smoke["command"], *smoke["args"]]:
            for rel in TESTSPEC_DIR_FILE.findall(arg):
                if not (spec_dir / rel).exists():
                    problems.append(f"smoke_run names {{testspec_dir}}/{rel}, which is missing")
    return problems


def validate_only(folders):
    bad = 0
    for folder in folders:
        for problem in spec_problems(folder):
            bad += 1
            print(f"{folder.name}: {problem}")
    print(f"{len(folders)} spec(s) checked: " + (f"{bad} problem(s)" if bad else "all valid"))
    return 1 if bad else 0


# --------------------------------------------------------------------------------------------
# reporting


def first_failure(result: SpecResult):
    for check in result.checks:
        if check.status == FAIL:
            return f"{check.stage}: {check.target}: {check.detail}"
    if result.result == SKIP:
        reasons = sorted({c.detail for c in result.checks if c.status == SKIP})
        return "; ".join(reasons)[:200]
    return ""


def print_table(results, verbose, stream=None):
    stream = stream or sys.stdout  # looked up per call, so a redirected stdout is honoured
    stages = ("artifacts", "needed", "pic", "smoke", "programs")
    width = max([len("PACKAGE")] + [len(r.package) for r in results])
    header = f"{'PACKAGE':<{width}}  RESULT  " + "  ".join(f"{s.upper():>9}" for s in stages) + "  NOTE"
    print(header, file=stream)
    print("-" * len(header), file=stream)
    for res in results:
        row = f"{res.package:<{width}}  {res.result:<6}  " + "  ".join(f"{res.counts(s):>9}" for s in stages)
        note = first_failure(res)
        print(row + ("  " + note[:160] if note else ""), file=stream)
        if verbose or res.result == FAIL:
            for check in res.checks:
                if verbose or check.status == FAIL:
                    print(f"{'':<{width}}    {check.status:<4} {check.stage:<9} {check.target}: {check.detail}",
                          file=stream)
    totals = {s: sum(r.result == s for r in results) for s in (PASS, FAIL, SKIP)}
    print("-" * len(header), file=stream)
    print(f"{len(results)} spec(s): {totals[PASS]} passed, {totals[FAIL]} failed, {totals[SKIP]} skipped", file=stream)


def parse_args(argv):
    ap = argparse.ArgumentParser(
        description="Check that native libraries and tools are installed correctly under a prefix.",
        epilog="Example: %(prog)s --prefix \"$CONDA_PREFIX\" -k zlib_ng -k 'lib*' --json report.json")
    ap.add_argument("--prefix", default="/usr/local", help="install prefix to check (default: /usr/local)")
    ap.add_argument("--specs-dir", default=str(HERE),
                    help="folder holding <package>/testspec.json (default: the folder of this script)")
    ap.add_argument("-k", "--package", action="append", default=[], metavar="NAME",
                    help="only run these packages; shell-style patterns allowed; repeatable")
    ap.add_argument("--with", dest="capabilities", action="append", default=[], metavar="CAP",
                    choices=sorted(CAPABILITIES | {"all"}),
                    help="enable checks that need this capability (gpu, cuda-driver, nvidia-driver, rdma, "
                         "toolchain, or all); repeatable")
    ap.add_argument("--cc", default=os.environ.get("CC", "cc"), help="C compiler (default: $CC or cc)")
    ap.add_argument("--cxx", default=os.environ.get("CXX", "c++"), help="C++ compiler (default: $CXX or c++)")
    ap.add_argument("--cuda-home", default=os.environ.get("CUDA_HOME", "/usr/local/cuda"),
                    help="value of the {cuda_home} placeholder (default: $CUDA_HOME or /usr/local/cuda)")
    ap.add_argument("--extra-cflags", default="", help="extra flags for every test program compile (one string)")
    ap.add_argument("--ld-path", action="store_true",
                    help="put the prefix lib dirs on LD_LIBRARY_PATH for smoke runs and test programs")
    ap.add_argument("--timeout-scale", type=float, default=1.0, help="multiply every timeout (slow machines)")
    ap.add_argument("-j", "--jobs", type=int, default=1, help="specs to run in parallel (default: 1)")
    ap.add_argument("--json", metavar="PATH", help="also write a JSON report here")
    ap.add_argument("--list", action="store_true", help="list the selected specs and exit")
    ap.add_argument("--validate-only", action="store_true",
                    help="only check that every selected spec is valid and every file it names exists; "
                         "needs no prefix and no compiler; exit 1 on any problem")
    ap.add_argument("--keep", action="store_true", help="keep each spec's build folder")
    ap.add_argument("-v", "--verbose", action="store_true", help="print every check, not only failures")
    args = ap.parse_args(argv)
    if args.timeout_scale <= 0:
        ap.error("--timeout-scale must be above 0")
    return args


def main(argv=None):
    args = parse_args(argv)
    specs_dir = Path(args.specs_dir).resolve()
    folders = discover(specs_dir, args.package)
    if not folders:
        print(f"no testspec.json matched under {specs_dir}", file=sys.stderr)
        return 2
    if args.list:
        for folder in folders:
            print(folder.name)
        return 0
    if args.validate_only:
        return validate_only(folders)
    prefix = Path(args.prefix).resolve()
    if not prefix.is_dir():
        print(f"prefix {prefix} is not a directory", file=sys.stderr)
        return 2
    caps = set(args.capabilities)
    if "all" in caps:
        caps = set(CAPABILITIES)
    opts = Options(prefix=prefix, cuda_home=Path(args.cuda_home), cc=args.cc, cxx=args.cxx, capabilities=caps,
                   extra_cflags=shlex.split(args.extra_cflags), ld_path=args.ld_path,
                   timeout_scale=args.timeout_scale, keep=args.keep, verbose=args.verbose)
    search = LibrarySearch(prefix)
    started = time.monotonic()
    if args.jobs > 1:
        with ThreadPoolExecutor(max_workers=args.jobs) as pool:
            results = list(pool.map(lambda f: run_spec(f, opts, search), folders))
    else:
        results = []
        for folder in folders:
            results.append(run_spec(folder, opts, search))
    print_table(results, args.verbose)
    if args.json:
        report = {
            "prefix": str(prefix),
            "specs_dir": str(specs_dir),
            "cc": opts.cc,
            "cxx": opts.cxx,
            "capabilities": sorted(caps),
            "seconds": round(time.monotonic() - started, 2),
            "summary": {s: sum(r.result == s for r in results) for s in (PASS, FAIL, SKIP)},
            "results": [dict(asdict(r), checks=[asdict(c) for c in r.checks]) for r in results],
        }
        Path(args.json).write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    return 1 if any(r.result == FAIL for r in results) else 0


if __name__ == "__main__":
    sys.exit(main())
