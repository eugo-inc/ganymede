// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Eugo, Inc.
// The ganymede website. Build it with `python3 tools/build_site.py build` from the repository's
// root: that command writes the pages (the hand-written pages of docs/, an index page per
// notebook folder and one Markdown page per notebook) into src/content/docs/, the generated
// settings into src/generated/site.json and the files the site serves as they are (the
// notebooks, their figures, the gallery, llms.txt) into .public/, then runs Astro and checks
// the built site.
import { readFileSync } from 'node:fs';
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

// The address of the site: every page and file lives under BASE, never at the root of the host.
const SITE = 'https://eugo.io';
const BASE = '/docs/ganymede/';

// The title, the description, the sidebar and the tags of every page's head, written by the
// prepare step from the folders of notebooks, the pages and the share image.
const generated = JSON.parse(
  readFileSync(new URL('./src/generated/site.json', import.meta.url), 'utf-8'),
);

export default defineConfig({
  site: SITE,
  base: BASE,
  trailingSlash: 'always',
  build: { format: 'directory' },
  // public/ holds the committed files (the favicon, the share image); the prepare step copies
  // them into .public/ together with the files it generates.
  publicDir: './.public',
  integrations: [
    starlight({
      title: generated.title,
      description: generated.description,
      sidebar: generated.sidebar,
      // The tags every page names the share image with: public/assets/social-preview.png,
      // drawn by tools/share_image.py, with its size and its alt text.
      head: generated.head,
      // The mark beside the name in the header, in the light and the dark theme, drawn by
      // tools/share_image.py (which also writes public/favicon.svg).
      logo: {
        light: './src/assets/mark-light.svg',
        dark: './src/assets/mark-dark.svg',
        alt: '',
      },
      // Inter, installed from the package and served by the site itself; the palette shared
      // with Eugo's other documentation; then what only this site needs.
      customCss: [
        '@fontsource-variable/inter',
        './src/styles/brand.css',
        './src/styles/ganymede.css',
      ],
      // The footer names the maker; the title of every page carries its GitHub and download links.
      components: {
        Footer: './src/components/Footer.astro',
        PageTitle: './src/components/PageTitle.astro',
      },
      social: [
        { icon: 'github', label: 'Source', href: 'https://github.com/eugo-inc/ganymede' },
      ],
      // A notebook's code cells have no file names: never read one from a first-line comment.
      expressiveCode: { frames: { extractFileNameFromCode: false } },
      // No page says when it changed, who changed it or where to edit it.
      lastUpdated: false,
      credits: false,
      // The 404 page is src/pages/404.astro.
      disable404Route: true,
    }),
  ],
});
