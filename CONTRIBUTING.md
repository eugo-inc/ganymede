# Contributing

Thanks for your interest in improving these examples and tests.

## Ground rules

- Open an issue first for anything larger than a small fix, so we can agree on the approach.
- Keep notebooks self-contained: each one should run on its own, top to bottom.
- Never commit credentials, hostnames or account ids. Notebooks that talk to a service read their
  connection settings from environment variables (see the note at the top of each such notebook).
- Commit `package_tests/` notebooks **without outputs**. The pattern notebooks under
  `ray_patterns/` are committed with outputs from a real run, so readers can see what each
  pattern produces.
- A demo notebook that draws plots or maps may be committed with its outputs (printed results,
  tables and static images) once its path is added to `FIGURE_NOTEBOOKS` in
  `tools/check_notebooks.py`. Save interactive (JavaScript) charts as PNG images, and drop
  warnings, `stderr` output and anything that shows a local path or address. Every other demo
  notebook is committed without outputs.
- Run the notebook checks before opening a pull request:

  ```bash
  python tools/check_notebooks.py
  ```

## Adding a package test

Add `package_tests/python/<package>.ipynb` with a few small examples that import the package
and exercise its main API. Tag cells that need extra capabilities (`requires-network`,
`requires-gpu`, …) as described in [package_tests/python/README.md](package_tests/python/README.md).

## Website

The website, <https://eugo.io/docs/ganymede/>, is built from this repository with Astro and
Starlight by `tools/build_site.py` (Python standard library only). The site itself needs Node.js
22.23.4 or newer and npm; the packages are installed exactly as `site/package-lock.json` pins them.

```bash
python3 tools/build_site.py build    # npm ci on the first run, then build and check site/dist/
python3 tools/build_site.py clean    # remove everything the build wrote; run it before every push
```

Where the pages come from:

- every notebook of `demo_notebooks/` and `ray_patterns/` is one page at its repository path,
  converted to Markdown with its outputs (it is never executed);
- every notebook folder has an index page: its `README.md`, or a generated list;
- `docs/index.md` (the home page), `docs/faq.md` and `docs/package_tests.md` are written by hand.
  On the home page, `<!-- linkcards -->`, `<!-- gallery -->` and `<!-- steps -->` mark the regions
  shown as link cards, the gallery of figures and numbered steps.

`build` fails on an Astro warning, a broken link or anchor, anything loaded from another host, and
any address or heading id of the old site (listed in `site/legacy-urls.txt`, which is never edited)
that the new site no longer serves. To look at the result, run `npx astro preview` in `site/`.
`tools/share_image.py` draws the mark, the favicon, the share image and the README's banner;
`python3 tools/share_image.py --check` says whether they are current. The palette in
`site/src/styles/brand.css` is shared with Eugo's other documentation sites:
`python3 tools/build_site.py check-brand --against PATH` compares it with another copy.

### Publishing

Every push to `main` runs `.github/workflows/site.yml`: it builds the site, uploads it to
eugo.io, reads it back, and only when the live site answers with the files it built does it
replace the old site at `https://eugo-inc.github.io/ganymede/` with pages that send each reader
to the same page at the new address. It needs the GitHub environment `eugo-docs`, limited to
deployments from `main`, with:

- secret `DOCS_AWS_ROLE_ARN`: the AWS role the job assumes through GitHub's OpenID Connect
  provider, which may write only under `docs/ganymede/` and drop cached copies;
- secret `DOCS_S3_BUCKET`: the bucket that holds the documentation sites;
- secret `DOCS_CLOUDFRONT_DISTRIBUTION_ID`: the distribution that serves eugo.io;
- variable `DOCS_AWS_REGION`: the region of that role's sign-in.

Without them the publish job uploads nothing and ends with a warning that names what is missing,
the redirect job does not run, and the old address keeps its site.

Replacing the old site is one-way until it is undone by hand. If <https://eugo.io/docs/ganymede/>
has to come down, run the `old-site` workflow (`.github/workflows/old-site.yml`) from `main`: it
builds the MkDocs site from the last commit that had it and puts it back at the old address. It
stays there while the new address fails its read-back; the first passing publish replaces it with
the redirect pages again.

### Uploading by hand

The fallback when the workflow cannot run. From a clean checkout of `main`, signed in to AWS with
a role that may write under `docs/ganymede/` of the documentation bucket:

```bash
python3 tools/build_site.py build
DOCS_S3_BUCKET='<the documentation bucket>'
DISTRIBUTION_ID='<the eugo.io distribution>'
target="s3://$DOCS_S3_BUCKET/docs/ganymede"
aws s3 sync site/dist/_astro/ "$target/_astro/" --size-only --no-follow-symlinks --only-show-errors \
  --cache-control 'public, max-age=31536000, immutable'
aws s3 sync site/dist/ "$target/" --delete --no-follow-symlinks --only-show-errors \
  --exclude '_astro/*' --exclude '*.md' --exclude '*.ipynb' --exclude '*.gz' \
  --cache-control 'public, max-age=300'
aws s3 sync site/dist/ "$target/" --delete --no-follow-symlinks --only-show-errors \
  --exclude '*' --include '*.ipynb' --content-type 'application/x-ipynb+json' \
  --cache-control 'public, max-age=300'
aws s3 sync site/dist/ "$target/" --delete --no-follow-symlinks --only-show-errors \
  --exclude '*' --include '*.gz' --content-type 'application/gzip' \
  --cache-control 'public, max-age=300'
aws s3 sync site/dist/ "$target/" --delete --no-follow-symlinks --only-show-errors \
  --exclude '*' --include '*.md' --content-type 'text/markdown; charset=utf-8' \
  --cache-control 'public, max-age=300'
id="$(aws cloudfront create-invalidation --distribution-id "$DISTRIBUTION_ID" \
  --paths '/docs/ganymede' '/docs/ganymede/*' --query Invalidation.Id --output text)"
aws cloudfront wait invalidation-completed --distribution-id "$DISTRIBUTION_ID" --id "$id"
curl -sS https://eugo.io/docs/ganymede/ | cmp - site/dist/index.html && echo "HOME AS BUILT"
curl -sS https://eugo.io/docs/ganymede/llms.txt | cmp - site/dist/llms.txt && echo "LLMS.TXT AS BUILT"
curl -sS -H 'Accept: text/markdown' https://eugo.io/docs/ganymede/faq/ | cmp - site/dist/faq/index.md && echo "MARKDOWN AS BUILT"
site='https://eugo.io/docs/ganymede/'
curl -sS -o /dev/null -w '%{http_code}\n' "${site}no-such-page/"    # 404
sleep 330    # pages still cached in browsers keep the hashed files they name
aws s3 sync site/dist/_astro/ "$target/_astro/" --delete --size-only --no-follow-symlinks \
  --only-show-errors --cache-control 'public, max-age=31536000, immutable'
python3 tools/build_site.py clean
```

Astro's hashed files go up first, so no page ever names one that is missing, and the old ones
are removed only at the end. Each `--delete` removes every key under its target and filter that
the build did not write, so never point these commands at another prefix. The three `AS BUILT` lines and `404` mean the site is live. The
host's one-time setup (the bucket policy, cache behaviours for `/docs/ganymede/*`, `/docs/ganymede`
and `/docs/404.html`, the role, and `site/hosting/docs-404.html` uploaded once as `docs/404.html`, the
page for an address under `/docs/` that no documentation site holds) is done by whoever runs eugo.io.

## License

By contributing you agree that your contributions are licensed under the
[BSD 3-Clause License](LICENSE).

If you adapt code from another project (its documentation, examples or tests), say so in a
comment or in the notebook's first cell, name the source and its license, and add the project
to [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). That code stays under its own license.
Only adapt code whose license allows it to be redistributed here (for example MIT, BSD or
Apache-2.0).
