/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for cmark_gfm: core API; calls cmark_gfm_core_extensions_ensure_registered(), cmark_parser_new(), cmark_find_syntax_extension(), cmark_parser_attach_syntax_extension().
 */

#include <cmark-gfm-core-extensions.h>
#include <cmark-gfm.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    cmark_gfm_core_extensions_ensure_registered();
    cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
    cmark_syntax_extension *table = cmark_find_syntax_extension("table");
    if (table == NULL || cmark_parser_attach_syntax_extension(parser, table) == 0) {
        printf("the table extension did not attach\n");
        return 1;
    }
    const char *text = "| a |\n| - |\n| **b** |\n";
    cmark_parser_feed(parser, text, strlen(text));
    cmark_node *document = cmark_parser_finish(parser);
    char *html = cmark_render_html(document, CMARK_OPT_DEFAULT, cmark_parser_get_syntax_extensions(parser));
    int ok = html != NULL && strstr(html, "<strong>b</strong>") != NULL && strstr(html, "<table>") != NULL;
    printf("%s\n", html == NULL ? "(null)" : html);
    free(html);
    cmark_node_free(document);
    cmark_parser_free(parser);
    return ok ? 0 : 1;
}
