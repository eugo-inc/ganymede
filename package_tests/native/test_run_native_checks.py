# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026, Eugo
"""Self-test for run_native_checks.py against a throwaway prefix built with the local C compiler.

Run from the repository root with:  python3 -m unittest package_tests/native/test_run_native_checks.py
or from package_tests/native with:  python3 -m unittest test_run_native_checks
(pytest collects the same tests.)
"""
from __future__ import annotations

import contextlib
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import run_native_checks as rnc  # noqa: E402

CC = os.environ.get("CC", "cc")
TRIPLET = rnc.TRIPLETS[0]

HEADER = """
#ifdef __cplusplus
extern "C" {
#endif
const char *demo_version(void);
int demo_value(void);
int multi_value(void);
#ifdef __cplusplus
}
#endif
"""
CLI = r"""
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "demo.h"
int main(int argc, char **argv) {
    const char *a = argc > 1 ? argv[1] : "";
    if (!strcmp(a, "--version")) { printf("demo-tool %s\n", demo_version()); return 0; }
    if (!strcmp(a, "--fail")) { fputs("demo-tool: asked to fail\n", stderr); return 3; }
    if (!strcmp(a, "--hang")) { sleep(30); return 0; }
    if (!strcmp(a, "--write")) {
        FILE *f = fopen("smoke_out.txt", "w");
        if (!f) return 4;
        fclose(f); puts("wrote"); return 0;
    }
    return demo_value() == 42 ? 0 : 1;
}
"""


def sh(*cmd, cwd=None):
    subprocess.run([str(c) for c in cmd], check=True, cwd=cwd, capture_output=True)


def run_main(argv):
    """Run the runner's main() and return (exit status, console text)."""
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        code = rnc.main(argv)
    return code, out.getvalue()


@unittest.skipUnless(sys.platform.startswith("linux") and shutil.which(CC) and shutil.which("ar"),
                     "needs Linux, a C compiler and ar")
class RunnerSelfTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(tempfile.mkdtemp(prefix="native_checks_selftest_"))
        prefix, src, specs, cwd = cls.root / "prefix", cls.root / "src", cls.root / "specs", cls.root / "cwd"
        lib, lib64, multi = prefix / "lib", prefix / "lib64", prefix / "lib" / TRIPLET
        for d in (lib, lib64, multi, prefix / "include", prefix / "bin", src, src / "ghost", src / "wrong", specs, cwd):
            d.mkdir(parents=True, exist_ok=True)
        (prefix / "include" / "demo.h").write_text(HEADER)
        (src / "demo.c").write_text('const char *demo_version(void) { return "1.0"; }\n'
                                    "int demo_value(void) { return 42; }\n")
        (src / "ext.c").write_text("int counter = 0;\nint bump(void) { return ++counter; }\n")
        (src / "multi.c").write_text("int multi_value(void) { return 7; }\n")
        (src / "helper.c").write_text("int helper(void) { return 1; }\n")
        (src / "user.c").write_text("int helper(void);\nint user(void) { return helper(); }\n")
        (src / "cli.c").write_text(CLI)

        # shared library with a soname and the usual symlinks
        sh(CC, "-shared", "-fPIC", "-Wl,-soname,libdemo.so.1", "-o", lib64 / "libdemo.so.1.0.0", src / "demo.c")
        os.symlink("libdemo.so.1.0.0", lib64 / "libdemo.so.1")
        os.symlink("libdemo.so.1", lib64 / "libdemo.so")
        os.symlink("missing.so.7", lib64 / "libdangling.so.7")
        # static archives: position-independent and not
        sh(CC, "-fPIC", "-c", src / "ext.c", "-o", src / "pic.o")
        sh("ar", "rcs", lib / "libpic.a", src / "pic.o")
        sh(CC, "-fno-pic", "-fno-PIE", "-c", src / "ext.c", "-o", src / "nopic.o")
        sh("ar", "rcs", lib / "libnopic.a", src / "nopic.o")
        # a compiled tool that finds libdemo through RUNPATH $ORIGIN/../lib64
        sh(CC, f"-I{prefix / 'include'}", src / "cli.c", "-o", prefix / "bin" / "demo-tool", f"-L{lib64}", "-ldemo",
           "-Wl,--enable-new-dtags", "-Wl,-rpath,$ORIGIN/../lib64")
        # a Debian-style multiarch library under lib/<triplet>/
        sh(CC, "-shared", "-fPIC", "-Wl,-soname,libmulti.so.0", "-o", multi / "libmulti.so.0.4.0", src / "multi.c")
        os.symlink("libmulti.so.0.4.0", multi / "libmulti.so.0")
        os.symlink("libmulti.so.0", multi / "libmulti.so")
        # NEEDED entries: one on a library that is gone, one on a file of another architecture,
        # one that only the prefix lib dirs satisfy
        sh(CC, "-shared", "-fPIC", "-Wl,-soname,libghost.so.9", "-o", src / "ghost" / "libghost.so", src / "helper.c")
        sh(CC, "-shared", "-fPIC", "-o", lib64 / "libneedsghost.so", src / "user.c", "-Wl,--no-as-needed",
           f"-L{src / 'ghost'}", "-lghost")
        sh(CC, "-shared", "-fPIC", "-Wl,-soname,libwrong.so.1", "-o", src / "wrong" / "libwrong.so", src / "helper.c")
        sh(CC, "-shared", "-fPIC", "-o", lib64 / "libneedswrong.so", src / "user.c", "-Wl,--no-as-needed",
           f"-L{src / 'wrong'}", "-lwrong", "-Wl,--enable-new-dtags", "-Wl,-rpath,$ORIGIN")
        blob = bytearray((src / "wrong" / "libwrong.so").read_bytes())
        machine = int.from_bytes(blob[18:20], "little")
        blob[18:20] = (62 if machine != 62 else 183).to_bytes(2, "little")  # x86-64 <-> AArch64
        (lib64 / "libwrong.so.1").write_bytes(bytes(blob))
        sh(CC, "-shared", "-fPIC", "-Wl,-soname,libhelper.so.1", "-o", lib64 / "libhelper.so.1", src / "helper.c")
        os.symlink("libhelper.so.1", lib64 / "libhelper.so")
        sh(CC, "-shared", "-fPIC", "-o", lib64 / "libneedshelper.so", src / "user.c", "-Wl,--no-as-needed",
           f"-L{lib64}", "-lhelper")

        def spec(name, body, files=None):
            folder = specs / name
            folder.mkdir()
            (folder / "testspec.json").write_text(json.dumps(dict(name=name, **body)))
            for fname, text in (files or {}).items():
                (folder / fname).write_text(textwrap.dedent(text))

        spec("good", {
            "kind": "shared-and-static-libs-with-cli",
            "expected_artifacts": ["bin/demo-tool", "lib/libdemo.so.1", "lib64/libdemo.so.1.2.3", "include/demo.h",
                                   "lib64/libpic.a", "lib/libmulti.so.0"],
            "expected_artifact_globs": ["lib64/libdemo.so*"],
            "static_archives_must_be_pic": True,
            "smoke_run": {"command": "bin/demo-tool", "args": ["--version"], "expect_stdout_contains": "demo-tool 1.0"},
            "test_programs": [
                {"source": "main.c", "link_with": ["demo", "multi"], "extra_cflags": ["-I{prefix}/include"]},
                {"source": "main.cpp", "language": "cpp", "link_with": ["demo"]},
                {"source": "main.c", "link_with": ["demo"], "requires": "gpu"},
            ],
        }, {"main.c": """
            #include <demo.h>
            #include <stdio.h>
            int main(void) { printf("demo %d OK\\n", demo_value()); return demo_value() == 42 && multi_value() == 7 ? 0 : 1; }
        """, "main.cpp": """
            #include <demo.h>
            #include <iostream>
            int main() { std::cout << "demo++ " << demo_version() << " OK\\n"; return demo_value() == 42 ? 0 : 1; }
        """})
        spec("bad", {
            "kind": "shared-and-static-libs",
            "expected_artifacts": ["lib64/libdangling.so.7", "lib/libabsent.so", "lib/libnopic.a",
                                   "lib64/libneedsghost.so", "lib64/libneedswrong.so", "lib64/libneedshelper.so"],
            "expected_artifact_globs": ["lib/nothing*.so"],
            "static_archives_must_be_pic": True,
            "smoke_run": {"command": "bin/demo-tool", "args": ["--version"], "expect_stdout_contains": "something else"},
            "test_programs": [{"source": "fails.c"}, {"source": "broken.c"},
                              {"source": "unlinked.c", "link_with": ["no_such_library_xyz"]},
                              {"source": "hangs.c", "runtime_timeout_s": 1}, {"source": "absent.c"}],
        }, {"fails.c": "int main(void) { return 3; }\n",
            "broken.c": "int main(void) { return undeclared_thing; }\n",
            "unlinked.c": "int nowhere(void);\nint main(void) { return nowhere(); }\n",
            "hangs.c": "#include <unistd.h>\nint main(void) { sleep(30); return 0; }\n"})
        spec("smoke_exit", {"kind": "cli-tool-only", "smoke_run": {"command": "bin/demo-tool", "args": ["--fail"]}})
        spec("smoke_missing", {"kind": "cli-tool-only", "smoke_run": {"command": "bin/not-there"}})
        spec("smoke_hang", {"kind": "cli-tool-only",
                            "smoke_run": {"command": "bin/demo-tool", "args": ["--hang"], "timeout_s": 1}})
        spec("smoke_cwd", {"kind": "cli-tool-only",
                           "smoke_run": {"command": "bin/demo-tool", "args": ["--write"], "expect_stdout_contains": "wrote"}})
        spec("gated", {"kind": "header-only", "requires": "rdma", "expected_artifacts": ["include/demo.h"]})
        spec("typo", {"kind": "header-only", "expected_artifact": ["include/demo.h"]})
        cls.prefix, cls.specs, cls.cwd = prefix, specs, cwd

        env_ld = os.environ.pop("LD_LIBRARY_PATH", None)
        old_cwd = os.getcwd()
        try:
            os.chdir(cwd)
            report = cls.root / "report.json"
            cls.exit_code, cls.console = run_main(["--prefix", str(prefix), "--specs-dir", str(specs),
                                                   "--json", str(report)])
            cls.results = {r["package"]: r for r in json.loads(report.read_text())["results"]}
            nocc = cls.root / "nocc.json"
            run_main(["--prefix", str(prefix), "--specs-dir", str(specs), "-k", "good", "-k", "bad",
                      "--cc", "/nonexistent/cc", "--cxx", "/nonexistent/c++", "--json", str(nocc)])
            cls.nocc = {r["package"]: r for r in json.loads(nocc.read_text())["results"]}
        finally:
            os.chdir(old_cwd)
            if env_ld is not None:
                os.environ["LD_LIBRARY_PATH"] = env_ld

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.root, ignore_errors=True)

    def rows(self, package, stage, results=None):
        results = results or self.results
        return {c["target"]: (c["status"], c["detail"]) for c in results[package]["checks"] if c["stage"] == stage}

    def test_exit_code_reports_failures(self):
        self.assertEqual(self.exit_code, 1)

    def test_good_spec_passes_every_check(self):
        good = self.results["good"]
        self.assertEqual(good["result"], "PASS", [c for c in good["checks"] if c["status"] == "FAIL"])
        artifacts = self.rows("good", "artifacts")
        self.assertTrue(all(s == "PASS" for s, _ in artifacts.values()))
        self.assertEqual(artifacts["lib/libdemo.so.1"][1], "found as lib64/libdemo.so.1")
        self.assertIn("same major version", artifacts["lib64/libdemo.so.1.2.3"][1])
        self.assertEqual(artifacts["lib/libmulti.so.0"][1], f"found as lib/{TRIPLET}/libmulti.so.0")
        needed = self.rows("good", "needed")
        self.assertEqual(needed["bin/demo-tool"][0], "PASS")
        self.assertNotIn("only in the prefix", needed["bin/demo-tool"][1])  # resolved through $ORIGIN
        self.assertEqual(self.rows("good", "pic"), {"lib/libpic.a": ("PASS", "links into a shared object")})
        self.assertEqual(self.rows("good", "smoke")["bin/demo-tool"][0], "PASS")
        self.assertEqual([c["status"] for c in good["checks"] if c["stage"] == "programs"], ["PASS", "PASS", "SKIP"])

    def test_bad_spec_fails_where_expected(self):
        artifacts = self.rows("bad", "artifacts")
        self.assertEqual(artifacts["lib64/libdangling.so.7"][0], "FAIL")
        self.assertEqual(artifacts["lib/libabsent.so"], ("FAIL", "missing"))
        self.assertEqual(artifacts["lib/nothing*.so"], ("FAIL", "no match"))
        self.assertEqual(self.rows("bad", "pic")["lib/libnopic.a"][0], "FAIL")
        self.assertEqual(self.rows("bad", "smoke")["bin/demo-tool"][0], "FAIL")

    def test_needed_entries(self):
        needed = self.rows("bad", "needed")
        self.assertEqual(needed["lib64/libneedsghost.so"], ("FAIL", "unresolved: libghost.so.9"))
        self.assertEqual(needed["lib64/libneedswrong.so"], ("FAIL", "unresolved: libwrong.so.1"))
        self.assertEqual(needed["lib64/libneedshelper.so"][0], "PASS")
        self.assertIn("only in the prefix lib dirs", needed["lib64/libneedshelper.so"][1])

    def test_program_failures(self):
        programs = self.rows("bad", "programs")
        self.assertTrue(programs["fails.c"][1].startswith("run exit 3"))
        self.assertTrue(programs["broken.c"][1].startswith("compile failed"))
        self.assertTrue(programs["unlinked.c"][1].startswith("link failed"))
        self.assertIn("timed out after 1s", programs["hangs.c"][1])
        self.assertEqual(programs["absent.c"][0], "FAIL")
        self.assertTrue(all(s == "FAIL" for s, _ in programs.values()))

    def test_smoke_failures(self):
        for package, text in (("smoke_exit", "exit 3"), ("smoke_missing", "command not found"),
                              ("smoke_hang", "timed out after 1s")):
            (status, detail), = self.rows(package, "smoke").values()
            self.assertEqual(status, "FAIL", package)
            self.assertIn(text, detail, package)

    def test_smoke_runs_in_a_scratch_folder(self):
        self.assertEqual(self.results["smoke_cwd"]["result"], "PASS")
        self.assertFalse((self.cwd / "smoke_out.txt").exists())

    def test_capability_gate_skips(self):
        self.assertEqual(self.results["gated"]["result"], "SKIP")

    def test_unknown_key_is_rejected(self):
        self.assertEqual(self.results["typo"]["result"], "FAIL")
        self.assertIn("unknown key", self.results["typo"]["checks"][0]["detail"])

    def test_without_a_compiler(self):
        # The relocation scan alone never passes an archive: it can only find absolute relocations.
        self.assertEqual(self.rows("good", "pic", self.nocc)["lib/libpic.a"][0], "SKIP")
        self.assertIn(self.rows("bad", "pic", self.nocc)["lib/libnopic.a"][0], ("SKIP", "FAIL"))
        self.assertEqual({c["status"] for c in self.nocc["good"]["checks"] if c["stage"] == "programs"}, {"SKIP"})
        self.assertEqual(self.nocc["good"]["result"], "PASS")

    def test_validate_only(self):
        code, out = run_main(["--validate-only", "--specs-dir", str(self.specs)])
        self.assertEqual(code, 1)
        self.assertIn("bad: test program source 'absent.c' is missing", out)
        self.assertIn("typo: invalid spec: unknown key", out)
        self.assertEqual(out.count("\n"), 3, out)
        code, out = run_main(["--validate-only", "--specs-dir", str(self.specs), "-k", "good", "-k", "smoke_*"])
        self.assertEqual(code, 0, out)


class PublicSpecsTest(unittest.TestCase):
    def test_every_public_spec_is_valid(self):
        code, out = run_main(["--validate-only"])
        self.assertEqual(code, 0, out)


if __name__ == "__main__":
    unittest.main()
