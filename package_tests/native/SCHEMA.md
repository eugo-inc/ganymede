# `testspec.json` reference

Each package folder holds one `testspec.json` and the test programs it names:

```
package_tests/native/
  run_native_checks.py
  <package>/
    testspec.json
    main.c                # test programs, data files and helper scripts the spec uses
    main_<aspect>.cpp
```

`run_native_checks.py` validates every spec before running it. An unknown key, a wrong
type or an unknown capability makes that spec FAIL with an `invalid spec` message, so a
typo never switches a check off quietly.

## Top-level fields

| Field | Type | Required | Default | Meaning |
|---|---|---|---|---|
| `name` | string | yes | | Package name. Must equal the folder name. |
| `kind` | string | yes | | What the package installs, one of the values below. Descriptive only: it selects no checks. |
| `expected_artifacts` | list of strings | no | `[]` | Paths relative to `--prefix` that must exist. |
| `expected_artifact_globs` | list of strings | no | `[]` | Glob patterns relative to `--prefix`. Each must match at least one file. `**` is allowed. |
| `static_archives_must_be_pic` | boolean | no | `false` | When true, every `.a` found through the two lists above must link into a shared object. |
| `smoke_run` | object or null | no | `null` | One command to run from the prefix. See below. |
| `test_programs` | list of objects | no | `[]` | C/C++ programs to compile, link and run. See below. |
| `requires` | string or null | no | `null` | A capability the whole spec needs. Without it the spec is skipped. |

### `kind` values

| Value | Typical contents |
|---|---|
| `shared-libs-only` | shared libraries, headers, pkg-config files |
| `shared-and-static-libs` | shared and static libraries, headers |
| `shared-and-static-libs-with-cli` | shared and static libraries plus command-line tools |
| `shared-libs-with-cli` | shared libraries plus command-line tools |
| `static-libs-only` | static libraries and headers |
| `header-only` | headers only |
| `cli-tool-only` | command-line tools only |

### How artifact rows are matched

- A row under `lib/` also matches the same path under `lib64/`, and the other way round.
  Both also match `lib/<multiarch triplet>/` (for example `lib/x86_64-linux-gnu/`), so a
  Debian-style `/usr` works.
- A versioned shared library row such as `lib/libfoo.so.1.2.3` that is not present
  matches another `libfoo.so.1.*` first and then any `libfoo.so.*`. The report says which
  file it found.
- A path that exists only as a dangling symlink is a failure.

## `smoke_run`

| Field | Type | Required | Default | Meaning |
|---|---|---|---|---|
| `command` | string | yes | | Program to run, relative to `--prefix` (for example `bin/zstd`), or an absolute path. |
| `args` | list of strings | no | `[]` | Arguments. Placeholders are expanded (see below). |
| `expect_stdout_contains` | string or null | no | `null` | Text that standard output must contain. `null` checks the exit status only. |
| `timeout_s` | number | no | `30` | Seconds before the run is stopped and counted as a failure. Above 0, at most 600. |
| `requires` | string or null | no | `null` | A capability this run needs. Without it only this run is skipped. |

The command must exit with status 0. `bin/` under the prefix is put first on `PATH`, and the
command runs in a scratch folder that is deleted afterwards, so files it writes are discarded.

## `test_programs[]`

| Field | Type | Required | Default | Meaning |
|---|---|---|---|---|
| `source` | string | yes | | File name beside `testspec.json`. |
| `language` | `"c"` or `"cpp"` | no | `"c"` | Selects `--cc` or `--cxx`. |
| `link_with` | list of strings | no | `[]` | Libraries, passed as `-l<name>`. A name containing `*`, `?` or `[` is a glob over `lib<name>.so` and `lib<name>.a` in the prefix's lib directories (see below): it is replaced by the first matching name in sorted order, or left as written when nothing matches, so the link step fails and names it. |
| `extra_cflags` | list of strings | no | `[]` | Extra compiler flags, placed before the defaults. Placeholders are expanded, and an `-I`, `-L` or `-isystem` path containing `*`, `?` or `[` is expanded as a glob. |
| `runtime_timeout_s` | number | no | `10` | Seconds the built program may run. Above 0, at most 300. |
| `compile_timeout_s` | number | no | `30` | Seconds the single compile-and-link command may take. Above 0, at most 300. |
| `requires` | string or null | no | `null` | A capability this program needs. Without it only this program is skipped. |

Each program is built with one command:

```
<cc or cxx> <extra_cflags> <--extra-cflags> -I<prefix>/include <source> -o <exe>
            -L<libdir> ... -Wl,-rpath,<libdir> ... -l<link_with> ...
```

where `<libdir>` is every existing one of `<prefix>/lib`, `<prefix>/lib64` and
`<prefix>/lib/<multiarch triplet>`. The program then runs in a scratch folder and must
exit with status 0.

A glob in `link_with` lets one spec work across library versions. For example
`"link_with": ["python3.[0-9]*"]` links `libpython3.12.so` on a prefix that has it and
`libpython3.13.so` on another, and never the stable-ABI `libpython3.so`. When several
versions match, the sorted-first one is used, which is the same one an `-I` glob such as
`-I{prefix}/include/python3.*` puts first.

## Placeholders

| Placeholder | Expands to | Used in |
|---|---|---|
| `{prefix}` | the `--prefix` value | `extra_cflags`, `smoke_run.command`, `smoke_run.args` |
| `{cuda_home}` | the `--cuda-home` value (default `$CUDA_HOME` or `/usr/local/cuda`) | same |
| `{testspec_dir}` | the absolute path of the spec's folder | same |

## Capabilities

`requires` takes one of these values. A check whose capability is not enabled is
reported as SKIP with the reason `needs --with <capability>`; it never counts as a pass.

| Capability | Meaning | How to enable |
|---|---|---|
| `gpu` | a usable NVIDIA GPU | `--with gpu` |
| `cuda-driver` | the CUDA driver library (`libcuda.so.1`) can be loaded | `--with cuda-driver` |
| `nvidia-driver` | the NVIDIA management library (`libnvidia-ml.so.1`) can be loaded | `--with nvidia-driver` |
| `rdma` | the kernel offers RDMA verbs | `--with rdma` |
| `toolchain` | a C or C++ compiler is available | automatic when `--cc` or `--cxx` is found, or `--with toolchain` |

`--with all` enables every capability.

## JSON Schema

The same rules, as a JSON Schema (draft 2020-12):

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "testspec.json",
  "type": "object",
  "additionalProperties": false,
  "required": ["name", "kind"],
  "$defs": {
    "capability": {"enum": ["gpu", "cuda-driver", "toolchain", "rdma", "nvidia-driver", null]},
    "strings": {"type": "array", "items": {"type": "string"}}
  },
  "properties": {
    "name": {"type": "string", "minLength": 1},
    "kind": {"enum": ["shared-libs-only", "shared-and-static-libs-with-cli", "shared-and-static-libs",
                      "shared-libs-with-cli", "static-libs-only", "header-only", "cli-tool-only"]},
    "expected_artifacts": {"$ref": "#/$defs/strings"},
    "expected_artifact_globs": {"$ref": "#/$defs/strings"},
    "static_archives_must_be_pic": {"type": "boolean"},
    "requires": {"$ref": "#/$defs/capability"},
    "smoke_run": {
      "type": ["object", "null"],
      "additionalProperties": false,
      "required": ["command"],
      "properties": {
        "command": {"type": "string", "minLength": 1},
        "args": {"$ref": "#/$defs/strings"},
        "expect_stdout_contains": {"type": ["string", "null"]},
        "timeout_s": {"type": "number", "exclusiveMinimum": 0, "maximum": 600},
        "requires": {"$ref": "#/$defs/capability"}
      }
    },
    "test_programs": {
      "type": "array",
      "items": {
        "type": "object",
        "additionalProperties": false,
        "required": ["source"],
        "properties": {
          "source": {"type": "string", "pattern": "^[^/]+$"},
          "language": {"enum": ["c", "cpp"]},
          "link_with": {"$ref": "#/$defs/strings"},
          "extra_cflags": {"$ref": "#/$defs/strings"},
          "runtime_timeout_s": {"type": "number", "exclusiveMinimum": 0, "maximum": 300},
          "compile_timeout_s": {"type": "number", "exclusiveMinimum": 0, "maximum": 300},
          "requires": {"$ref": "#/$defs/capability"}
        }
      }
    }
  }
}
```

## Example

```json
{
  "name": "zstd",
  "kind": "shared-libs-with-cli",
  "expected_artifacts": ["bin/zstd", "lib/libzstd.so.1", "include/zstd.h"],
  "expected_artifact_globs": ["lib/pkgconfig/libzstd.pc"],
  "static_archives_must_be_pic": false,
  "smoke_run": {"command": "bin/zstd", "args": ["--version"], "expect_stdout_contains": null},
  "test_programs": [
    {"source": "main.c", "language": "c", "link_with": ["zstd"]}
  ]
}
```
