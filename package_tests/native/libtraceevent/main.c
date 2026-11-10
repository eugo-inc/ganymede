/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for libtraceevent: core API; calls trace_seq_init(), tep_print_plugins(), trace_seq_terminate(), strtok().
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <traceevent/event-parse.h>
#include <traceevent/trace-seq.h>

#define PLUGIN_FOLDER "/usr/local/lib64/traceevent/plugins/"
#define EXPECTED_PLUGIN_COUNT 13

static const char SCHED_SWITCH_FORMAT[] =
    "name: sched_switch\n"
    "ID: 316\n"
    "format:\n"
    "\tfield:unsigned short common_type;\toffset:0;\tsize:2;\tsigned:0;\n"
    "\tfield:unsigned char common_flags;\toffset:2;\tsize:1;\tsigned:0;\n"
    "\tfield:unsigned char common_preempt_count;\toffset:3;\tsize:1;\tsigned:0;\n"
    "\tfield:int common_pid;\toffset:4;\tsize:4;\tsigned:1;\n"
    "\n"
    "\tfield:char prev_comm[16];\toffset:8;\tsize:16;\tsigned:0;\n"
    "\tfield:pid_t prev_pid;\toffset:24;\tsize:4;\tsigned:1;\n"
    "\tfield:int prev_prio;\toffset:28;\tsize:4;\tsigned:1;\n"
    "\tfield:long prev_state;\toffset:32;\tsize:8;\tsigned:1;\n"
    "\tfield:char next_comm[16];\toffset:40;\tsize:16;\tsigned:0;\n"
    "\tfield:pid_t next_pid;\toffset:56;\tsize:4;\tsigned:1;\n"
    "\tfield:int next_prio;\toffset:60;\tsize:4;\tsigned:1;\n"
    "\n"
    "print fmt: \"prev_comm=%s prev_pid=%d prev_prio=%d ==> next_comm=%s next_pid=%d next_prio=%d\", "
    "REC->prev_comm, REC->prev_pid, REC->prev_prio, REC->next_comm, REC->next_pid, REC->next_prio\n";

static const char EXPECTED_PLUGIN_OUTPUT[] = "swapper:0 [120] R ==> perf:1234 [120]";

static int count_plugins(const struct tep_plugin_list *plugins) {
    struct trace_seq names;
    int count = 0;

    trace_seq_init(&names);
    tep_print_plugins(&names, "", "\n", plugins);
    trace_seq_terminate(&names);
    for (char *line = strtok(names.buffer, "\n"); line != NULL; line = strtok(NULL, "\n")) {
        if (strncmp(line, PLUGIN_FOLDER, strlen(PLUGIN_FOLDER)) != 0) {
            fprintf(stderr, "plugin loaded from outside %s: %s\n", PLUGIN_FOLDER, line);
            trace_seq_destroy(&names);
            return -1;
        }
        count++;
    }
    trace_seq_destroy(&names);
    return count;
}

int main(void) {
    unsetenv("TRACEEVENT_PLUGIN_DIR");
    setenv("HOME", "/nonexistent-demo-libtraceevent-test", 1);

    struct tep_handle *tep = tep_alloc();
    if (tep == NULL) {
        fprintf(stderr, "tep_alloc failed\n");
        return 1;
    }
    tep_set_long_size(tep, (int)sizeof(long));

    struct tep_plugin_list *plugins = tep_load_plugins(tep);
    int plugin_count = count_plugins(plugins);
    if (plugin_count != EXPECTED_PLUGIN_COUNT) {
        fprintf(stderr, "loaded %d event plugins from the compiled-in folder, expected %d\n", plugin_count,
                EXPECTED_PLUGIN_COUNT);
        return 1;
    }

    if (tep_parse_event(tep, SCHED_SWITCH_FORMAT, sizeof(SCHED_SWITCH_FORMAT) - 1, "sched") != 0) {
        fprintf(stderr, "tep_parse_event could not parse the sched_switch format\n");
        return 1;
    }
    struct tep_event *event = tep_find_event_by_name(tep, "sched", "sched_switch");
    if (event == NULL || event->id != 316 || tep_find_field(event, "next_pid") == NULL ||
        tep_find_field(event, "prev_comm") == NULL) {
        fprintf(stderr, "the parsed sched_switch event is missing its id or fields\n");
        return 1;
    }

    unsigned char data[64] = {0};
    unsigned short type = 316;
    int prev_pid = 0, prio = 120, next_pid = 1234;
    long prev_state = 0;
    memcpy(data + 0, &type, sizeof(type));
    memcpy(data + 8, "swapper", 7);
    memcpy(data + 24, &prev_pid, sizeof(prev_pid));
    memcpy(data + 28, &prio, sizeof(prio));
    memcpy(data + 32, &prev_state, sizeof(prev_state));
    memcpy(data + 40, "perf", 4);
    memcpy(data + 56, &next_pid, sizeof(next_pid));
    memcpy(data + 60, &prio, sizeof(prio));
    struct tep_record record = {.size = (int)sizeof(data), .data = data, .cpu = 0};

    struct trace_seq printed;
    trace_seq_init(&printed);
    tep_print_event(tep, &printed, &record, "%s", TEP_PRINT_INFO);
    trace_seq_terminate(&printed);
    if (strstr(printed.buffer, EXPECTED_PLUGIN_OUTPUT) == NULL) {
        fprintf(stderr, "the sched_switch record printed as '%s', not through the sched_switch plugin ('%s')\n",
                printed.buffer, EXPECTED_PLUGIN_OUTPUT);
        trace_seq_destroy(&printed);
        return 1;
    }
    printf("libtraceevent: %d plugins from %s; sched_switch printed by its plugin: %s\n", plugin_count, PLUGIN_FOLDER,
           printed.buffer);
    trace_seq_destroy(&printed);
    tep_unload_plugins(plugins, tep);
    tep_free(tep);
    return 0;
}
