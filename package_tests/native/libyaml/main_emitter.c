/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libyaml: emitter; calls yaml_scalar_event_initialize(), yaml_emitter_emit(), yaml_emitter_initialize(), yaml_emitter_set_output_string().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <yaml.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int emit_scalar(yaml_emitter_t *emitter, const char *value) {
    yaml_event_t event;
    if (yaml_scalar_event_initialize(&event,
                                     NULL,
                                     NULL,
                                     (yaml_char_t *)value,
                                     (int)strlen(value),
                                     1,
                                     1,
                                     YAML_PLAIN_SCALAR_STYLE) == 0) {
        return 0;
    }
    return yaml_emitter_emit(emitter, &event);
}

int main(void) {
    unsigned char buffer[4096];
    size_t written = 0;

    yaml_emitter_t emitter;
    assert(yaml_emitter_initialize(&emitter) == 1);
    yaml_emitter_set_output_string(&emitter, buffer, sizeof(buffer), &written);
    yaml_emitter_set_canonical(&emitter, 0);
    yaml_emitter_set_unicode(&emitter, 1);

    yaml_event_t event;

    assert(yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(yaml_mapping_start_event_initialize(&event,
                                               NULL,
                                               NULL,
                                               1,
                                               YAML_BLOCK_MAPPING_STYLE) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(emit_scalar(&emitter, "color") == 1);
    assert(emit_scalar(&emitter, "blue") == 1);

    assert(yaml_mapping_end_event_initialize(&event) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(yaml_document_end_event_initialize(&event, 1) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(yaml_stream_end_event_initialize(&event) == 1);
    assert(yaml_emitter_emit(&emitter, &event) == 1);

    assert(yaml_emitter_flush(&emitter) == 1);
    yaml_emitter_delete(&emitter);

    assert(written > 0);
    assert(written < sizeof(buffer));

    buffer[written] = '\0';
    assert(strstr((const char *)buffer, "color") != NULL);
    assert(strstr((const char *)buffer, "blue") != NULL);

    printf("libyaml emitter test OK: emitted %zu bytes\n", written);
    return 0;
}
