// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2026, Eugo, Inc.
// The pages (written into src/content/docs/ by the prepare step) and the interface texts.
// Each page may name its source on GitHub, the notebook a reader can download, and its kind.
import { defineCollection } from 'astro:content';
import { z } from 'astro/zod';
import { docsLoader, i18nLoader } from '@astrojs/starlight/loaders';
import { docsSchema, i18nSchema } from '@astrojs/starlight/schema';

export const collections = {
  docs: defineCollection({
    loader: docsLoader(),
    schema: docsSchema({
      extend: z.object({
        // The page's source on GitHub: the notebook, the README or the Markdown file.
        github: z.string().url().optional(),
        // The address of the notebook to download, under the site's base.
        download: z.string().optional(),
        kind: z.enum(['doc', 'section', 'notebook']).optional(),
        // The id the page's title had on the MkDocs site, kept so that old links to it still land.
        legacyTitleId: z.string().optional(),
      }),
    }),
  }),
  i18n: defineCollection({ loader: i18nLoader(), schema: i18nSchema() }),
};
