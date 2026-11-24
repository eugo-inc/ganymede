# Third-party notices

Original content in this repository is released under the [BSD 3-Clause License](LICENSE),
Copyright (c) 2026, Eugo, Inc. Some notebooks contain code adapted from the documentation,
README, examples or tests of the package they demonstrate or test. That code stays under
its upstream license. This file lists each upstream project, where its code is used, its
copyright notice and its license; the full license texts are at the end.

How adapted code is marked:

- In `package_tests/python/`, an adapted cell names its source in a comment that starts
  with `Adapted from` or `Source:`.
- In `demo_notebooks/`, the first cell of an adapted notebook has a *Source and license*
  entry. Many other demos follow the usage shown in their library's documentation, which
  each notebook links under *Resources*; see [demo_notebooks/README.md](demo_notebooks/README.md).

## Package tests (`package_tests/python/`)

### Mooncake

- Used in: `package_tests/python/mooncake_transfer_engine.ipynb`
- Source: <https://github.com/kvcache-ai/Mooncake>, release v0.3.13.post1:
  `mooncake-wheel/tests/transfer_engine_target.py`,
  `mooncake-wheel/tests/transfer_engine_initiator_test.py`,
  `mooncake-wheel/tests/test_distributed_object_store.py`,
  `mooncake-wheel/tests/test_transfer_on_cuda.py`
- License: Apache-2.0
- Copyright: the Mooncake authors

### LMCache

- Used in: `package_tests/python/lmcache.ipynb`
- Source: <https://github.com/LMCache/LMCache>, release v0.5.5:
  `tests/v1/lmcache_native/test_bitmap.py`, `tests/v1/test_address_manager.py`
- License: Apache-2.0
- Copyright: the LMCache authors

### NIXL

- Used in: `package_tests/python/nixl.ipynb`, `package_tests/python/nixl_cu13.ipynb`
- Source: <https://github.com/ai-dynamo/nixl>, release v1.4.1:
  `examples/python/basic_two_peers.py`, `examples/python/nixl_gds_example.py`
- License: Apache-2.0
- Copyright (c) 2025-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

### Apache TVM FFI

- Used in: `package_tests/python/apache_tvm_ffi.ipynb`
- Source: <https://github.com/apache/tvm-ffi>: `tests/python/test_libinfo.py`,
  `tests/python/test_function.py` (`test_global_func`),
  `tests/python/test_tensor.py` (`test_tensor_auto_dlpack`)
- License: Apache-2.0
- NOTICE of the upstream project:

  ```text
  Apache TVM FFI
  Copyright 2024-present The Apache Software Foundation

  This product includes software developed at
  The Apache Software Foundation (http://www.apache.org/).
  ```

### Icechunk

- Used in: `package_tests/python/icechunk.ipynb` (modified: in-memory storage, small arrays, added assertions)
- Source: <https://icechunk.io>: the quickstart and version-control guides
- License: Apache-2.0
- Copyright: the Icechunk authors

### VirtualiZarr

- Used in: `package_tests/python/virtualizarr.ipynb`
- Source: <https://github.com/zarr-developers/VirtualiZarr>: docstrings and tests in the `virtualizarr` package
- License: Apache-2.0
- Copyright: the VirtualiZarr authors

### obspec-utils

- Used in: `package_tests/python/obspec_utils.ipynb`
- Source: <https://github.com/developmentseed/obspec-utils>: docstrings in the `obspec_utils` package
- License: Apache-2.0
- Copyright: the obspec-utils authors

### fastdownload

- Used in: `package_tests/python/fastdownload.ipynb`
- Source: <https://github.com/fastai/fastdownload>: the README example
- License: Apache-2.0
- Copyright: the fastdownload authors

### humming-kernels

- Used in: `package_tests/python/humming_kernels.ipynb`
- Source: the humming-kernels README usage example (release 0.1.16)
- License: Apache-2.0
- Copyright: the humming-kernels authors

### prefab-ui

- Used in: `package_tests/python/prefab_ui.ipynb`
- Source: the prefab-ui README
- License: Apache-2.0
- Copyright: the prefab-ui authors

### obspec

- Used in: `package_tests/python/obspec.ipynb`
- Source: <https://github.com/developmentseed/obspec>: docstrings in `obspec/_list.py` and `obspec/exceptions.py`
- License: MIT
- Copyright (c) 2025 Development Seed

### TileLang

- Used in: `package_tests/python/tilelang.ipynb` (modified: condensed into one cell; prints which run-time compiler built the kernel)
- Source: <https://github.com/tile-ai/tilelang>: the README quick-start, release v0.1.15
- License: MIT
- Copyright (c) Tile-AI.

### langchain-protocol

- Used in: `package_tests/python/langchain_protocol.ipynb`
- Source: the langchain-protocol README usage example
- License: MIT
- Copyright (c) 2026 LangChain, Inc.

### pyvers

- Used in: `package_tests/python/pyvers.ipynb`
- Source: <https://github.com/vmoens/pyvers>: the README and `pyvers/implement_for.py` (`check_version`)
- License: MIT
- Copyright (c) 2025 Vincent Moens

### DeepGEMM

- Used in: `package_tests/python/deep_gemm.ipynb` (the `calc_diff` formula)
- Source: <https://github.com/deepseek-ai/DeepGEMM>: the tests
- License: MIT
- Copyright (c) 2025 DeepSeek

## Demo notebooks (`demo_notebooks/`)

| Project | Used in | Source | Copyright | License |
| --- | --- | --- | --- | --- |
| asyncpg | `demo_notebooks/data_storage/databases/postgres/asyncpg.ipynb` | The asyncpg README and documentation, <https://github.com/MagicStack/asyncpg> | Copyright (C) 2016-present the asyncpg authors and contributors | Apache-2.0 |
| Bokeh | `demo_notebooks/data_visualization/bokeh/bokeh.ipynb` | The Bokeh first steps guide, <https://docs.bokeh.org/en/latest/docs/first_steps.html> | Copyright (c) Anaconda, Inc., and Bokeh Contributors | BSD-3-Clause |
| Cartopy | `demo_notebooks/data_visualization/cartopy.ipynb` | The Cartopy example gallery, <https://cartopy.readthedocs.io/stable/gallery/index.html> | Copyright (c) Crown and Cartopy contributors | BSD-3-Clause |
| descartes | `demo_notebooks/geospatial/descartes.ipynb` | The descartes README, <https://pypi.org/project/descartes/> | Copyright Sean Gillies | BSD (see note below) |
| GeoAlchemy 2 | `demo_notebooks/data_storage/databases/postgres/geoalchemy2.ipynb` | The GeoAlchemy 2 ORM tutorial, <https://geoalchemy-2.readthedocs.io/en/stable/orm_tutorial.html> | Copyright (c) 2013 Eric Lemoine | MIT |
| GeoViews | `demo_notebooks/data_visualization/holoviews/geoviews/geoviews.ipynb` | The GeoViews documentation and examples, <https://geoviews.org/> | Copyright (c) 2019, HoloViz Developers | BSD-3-Clause |
| google-auth-oauthlib | `demo_notebooks/geospatial/google/authentication.ipynb` | The google-auth-oauthlib documentation (the custom `Flow` example), <https://google-auth-oauthlib.readthedocs.io/> | Copyright Google LLC | Apache-2.0 |
| h5netcdf | `demo_notebooks/geospatial/h5netcdf.ipynb` | The h5netcdf README, <https://github.com/h5netcdf/h5netcdf> | Copyright (c) 2015, h5netcdf developers | BSD-3-Clause |
| HoloViews | `demo_notebooks/data_visualization/holoviews/holoviews/holoviews.ipynb` | The HoloViews gallery (the Lorenz attractor demo), <https://holoviews.org/gallery/index.html> | Copyright (c) 2005, holoviz | BSD-3-Clause |
| jsonschema | `demo_notebooks/data_structures/jsonschema.ipynb` | The jsonschema README, <https://github.com/python-jsonschema/jsonschema> | Copyright (c) 2013 Julian Berman | MIT |
| MSAL for Python | `demo_notebooks/geospatial/microsoft/azure/msal.ipynb` | The MSAL for Python README, <https://github.com/AzureAD/microsoft-authentication-library-for-python> | Copyright (c) Microsoft Corporation | MIT |
| pydantic | `demo_notebooks/data_processing/pydantic.ipynb` | The pydantic README, <https://github.com/pydantic/pydantic> | Copyright (c) 2017 to present Pydantic Services Inc. and individual contributors | MIT |
| python-devtools | `demo_notebooks/utilities/devtools.ipynb` | The python-devtools documentation, <https://python-devtools.helpmanual.io/usage/> | Copyright (c) 2017 to present Samuel Colvin | MIT |
| python-oracledb | `demo_notebooks/data_storage/databases/oracle/oracledb.ipynb` | The python-oracledb samples, <https://github.com/oracle/python-oracledb/tree/main/samples> | Copyright (c) 2016, 2026 Oracle and/or its affiliates | UPL-1.0 OR Apache-2.0 (used under Apache-2.0) |
| scikit-learn | `demo_notebooks/data_processing/scikit_learn.ipynb` | The scikit-learn documentation (Getting Started and the Naive Bayes user guide), <https://scikit-learn.org/stable/> | Copyright (c) 2007-2026 The scikit-learn developers | BSD-3-Clause |
| SciPy | `demo_notebooks/data_processing/scipy.ipynb` | The SciPy tutorial on special functions (the vibrating drum head), <https://docs.scipy.org/doc/scipy/tutorial/special.html> | Copyright (c) 2001-2002 Enthought, Inc. 2003, SciPy Developers | BSD-3-Clause |
| spglm | `demo_notebooks/geospatial/pysal/spglm.ipynb` | spglm's README and tests (the `y` and `X` arrays are the data of `TestBinomial` in `spglm/tests/test_glm.py`), <https://github.com/pysal/spglm> | Copyright (c) 2018, PySAL - spglm developers | BSD-3-Clause |
| SQLAlchemy | `demo_notebooks/data_storage/databases/postgres/sqlalchemy.ipynb`<br>`demo_notebooks/data_storage/databases/mysql/sqlalchemy.ipynb`<br>`demo_notebooks/data_storage/databases/sqllite/sqlalchemy.ipynb` | The SQLAlchemy documentation (asyncio extension and ORM tutorial), <https://docs.sqlalchemy.org/en/20/orm/extensions/asyncio.html> | Copyright 2005-2026 SQLAlchemy authors and contributors | MIT |

Notes:

- python-oracledb ships a NOTICE file that reads: `Copyright (c) 2016, 2026, Oracle and/or its affiliates.`
- descartes declares the BSD license in its package metadata but ships no license text, so only its
  copyright holder is given here.

## Website (`site/`)

The website, <https://eugo.io/docs/ganymede/>, is built with these packages, installed exactly as
`site/package-lock.json` pins them. The built site serves the parts of them that run in a reader's
browser: Astro's and Starlight's scripts and styles, Expressive Code's code-block styles and
scripts, Pagefind's search, and the Inter font files.

| Project | Version | Source | Copyright | License |
| --- | --- | --- | --- | --- |
| Astro | 7.4.4 | <https://github.com/withastro/astro> | Copyright (c) 2021 Fred K. Schott; parts: Copyright (c) 2020 the SvelteKit contributors | MIT |
| Starlight | 0.42.8 | <https://github.com/withastro/starlight> | Copyright (c) 2023 Astro contributors | MIT |
| Expressive Code | 0.44.3 | <https://github.com/expressive-code/expressive-code> | Copyright (c) 2023 Tibor Schiemann | MIT |
| Pagefind | 1.6.0 | <https://github.com/Pagefind/pagefind> | Copyright (c) Pagefind | MIT |
| Inter, packaged as `@fontsource-variable/inter` | 5.3.0 | <https://github.com/rsms/inter> | Copyright 2016 The Inter Project Authors | OFL-1.1 |

## License texts

### Apache License 2.0

```text
                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```

### MIT License

Applies, with the copyright notices listed above, to the MIT-licensed projects in this file.

```text
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

### BSD 3-Clause License

Applies, with the copyright notices listed above, to the BSD-3-Clause projects in this file.

```text
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### SIL Open Font License 1.1

Applies to the Inter font that the website serves.

```text
-----------------------------------------------------------
SIL OPEN FONT LICENSE Version 1.1 - 26 February 2007
-----------------------------------------------------------

PREAMBLE
The goals of the Open Font License (OFL) are to stimulate worldwide
development of collaborative font projects, to support the font creation
efforts of academic and linguistic communities, and to provide a free and
open framework in which fonts may be shared and improved in partnership
with others.

The OFL allows the licensed fonts to be used, studied, modified and
redistributed freely as long as they are not sold by themselves. The
fonts, including any derivative works, can be bundled, embedded,
redistributed and/or sold with any software provided that any reserved
names are not used by derivative works. The fonts and derivatives,
however, cannot be released under any other type of license. The
requirement for fonts to remain under this license does not apply
to any document created using the fonts or their derivatives.

DEFINITIONS
"Font Software" refers to the set of files released by the Copyright
Holder(s) under this license and clearly marked as such. This may
include source files, build scripts and documentation.

"Reserved Font Name" refers to any names specified as such after the
copyright statement(s).

"Original Version" refers to the collection of Font Software components as
distributed by the Copyright Holder(s).

"Modified Version" refers to any derivative made by adding to, deleting,
or substituting -- in part or in whole -- any of the components of the
Original Version, by changing formats or by porting the Font Software to a
new environment.

"Author" refers to any designer, engineer, programmer, technical
writer or other person who contributed to the Font Software.

PERMISSION & CONDITIONS
Permission is hereby granted, free of charge, to any person obtaining
a copy of the Font Software, to use, study, copy, merge, embed, modify,
redistribute, and sell modified and unmodified copies of the Font
Software, subject to the following conditions:

1) Neither the Font Software nor any of its individual components,
in Original or Modified Versions, may be sold by itself.

2) Original or Modified Versions of the Font Software may be bundled,
redistributed and/or sold with any software, provided that each copy
contains the above copyright notice and this license. These can be
included either as stand-alone text files, human-readable headers or
in the appropriate machine-readable metadata fields within text or
binary files as long as those fields can be easily viewed by the user.

3) No Modified Version of the Font Software may use the Reserved Font
Name(s) unless explicit written permission is granted by the corresponding
Copyright Holder. This restriction only applies to the primary font name as
presented to the users.

4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font
Software shall not be used to promote, endorse or advertise any
Modified Version, except to acknowledge the contribution(s) of the
Copyright Holder(s) and the Author(s) or with their explicit written
permission.

5) The Font Software, modified or unmodified, in part or in whole,
must be distributed entirely under this license, and must not be
distributed under any other license. The requirement for fonts to
remain under this license does not apply to any document created
using the Font Software.

TERMINATION
This license becomes null and void if any of the above conditions are
not met.

DISCLAIMER
THE FONT SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM
OTHER DEALINGS IN THE FONT SOFTWARE.
```
