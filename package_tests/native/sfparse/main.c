/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for sfparse: core API; calls sfparse_parser_init(), sfparse_parser_dict().
 */

#include <sfparse.h>

#include <stdio.h>
#include <string.h>

int main(void) {
  const char *value = "u=3, i";
  sfparse_parser sfp;
  sfparse_vec key;
  sfparse_value item;
  int saw_urgency = 0, saw_incremental = 0;

  sfparse_parser_init(&sfp, (const uint8_t *)value, strlen(value));
  for (;;) {
    int rv = sfparse_parser_dict(&sfp, &key, &item);
    if (rv == SFPARSE_ERR_EOF) {
      break;
    }
    if (rv != 0) {
      fprintf(stderr, "sfparse_parser_dict failed: %d\n", rv);
      return 1;
    }
    if (key.len == 1 && key.base[0] == 'u') {
      if (item.type != SFPARSE_TYPE_INTEGER || item.integer != 3) {
        fprintf(stderr, "u is not the integer 3\n");
        return 1;
      }
      saw_urgency = 1;
    } else if (key.len == 1 && key.base[0] == 'i') {
      if (item.type != SFPARSE_TYPE_BOOLEAN || item.boolean != 1) {
        fprintf(stderr, "i is not the boolean true\n");
        return 1;
      }
      saw_incremental = 1;
    }
  }
  if (!saw_urgency || !saw_incremental) {
    fprintf(stderr, "missing key: u=%d i=%d\n", saw_urgency, saw_incremental);
    return 1;
  }
  printf("sfparse OK: u=3, i=true\n");
  return 0;
}
