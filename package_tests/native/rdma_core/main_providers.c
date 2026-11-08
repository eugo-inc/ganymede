/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for rdma_core: providers; dlopens every provider plugin that a .driver file in
 * etc/libibverbs.d names.
 */

#define _GNU_SOURCE
#include <infiniband/verbs.h>

#include <dirent.h>
#include <dlfcn.h>
#include <glob.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failure_count = 0;
static int is_mlx5_named = 0;
static int is_efa_named = 0;

static void load_provider(const char *provider_dir, const char *driver_file, const char *name) {
    char pattern[PATH_MAX];
    glob_t matches;
    if (name[0] == '/') {
        snprintf(pattern, sizeof(pattern), "%s-rdmav*.so", name);
    } else {
        snprintf(pattern, sizeof(pattern), "%s/lib%s-rdmav*.so", provider_dir, name);
    }
    if (glob(pattern, 0, NULL, &matches) != 0 || matches.gl_pathc == 0) {
        fprintf(stderr, "%s names provider '%s', but nothing matches %s\n", driver_file, name, pattern);
        ++failure_count;
        return;
    }
    for (size_t i = 0; i < matches.gl_pathc; ++i) {
        void *handle = dlopen(matches.gl_pathv[i], RTLD_NOW | RTLD_LOCAL);
        if (handle == NULL) {
            fprintf(stderr, "provider '%s' did not load: %s\n", name, dlerror());
            ++failure_count;
        } else {
            printf("loaded provider %s (%s)\n", name, matches.gl_pathv[i]);
        }
    }
    globfree(&matches);
}

/* Same grammar as read_config_file: `driver <name>` lines; blank lines and `#` comments skipped. */
static int read_driver_file(const char *provider_dir, const char *driver_file) {
    FILE *file = fopen(driver_file, "r");
    char line[512];
    int driver_line_count = 0;
    if (file == NULL) {
        fprintf(stderr, "cannot read %s\n", driver_file);
        ++failure_count;
        return 0;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        char *saveptr = NULL;
        char *keyword = strtok_r(line, " \t\n", &saveptr);
        if (keyword == NULL || keyword[0] == '#') {
            continue;
        }
        char *name = strtok_r(NULL, " \t\n", &saveptr);
        if (strcmp(keyword, "driver") != 0 || name == NULL) {
            fprintf(stderr, "%s: unexpected line starting '%s'\n", driver_file, keyword);
            ++failure_count;
            continue;
        }
        ++driver_line_count;
        is_mlx5_named |= strcmp(name, "mlx5") == 0;
        is_efa_named |= strcmp(name, "efa") == 0;
        load_provider(provider_dir, driver_file, name);
    }
    fclose(file);
    return driver_line_count;
}

int main(void) {
    Dl_info library_info;
    if (dladdr((void *)&ibv_get_device_list, &library_info) == 0 || library_info.dli_fname == NULL) {
        fprintf(stderr, "dladdr could not locate libibverbs\n");
        return 1;
    }
    char library_path[PATH_MAX];
    if (realpath(library_info.dli_fname, library_path) == NULL) {
        fprintf(stderr, "cannot resolve %s\n", library_info.dli_fname);
        return 1;
    }
    char library_dir[PATH_MAX];
    snprintf(library_dir, sizeof(library_dir), "%s", dirname(library_path));
    char provider_dir[PATH_MAX];
    char config_dir[PATH_MAX];
    snprintf(provider_dir, sizeof(provider_dir), "%s/libibverbs", library_dir);

    /* The install prefix is the library dir minus lib/, lib64/ or lib/<multiarch triplet>/.
     * Provider configs live in <prefix>/etc/libibverbs.d, or /etc/libibverbs.d when the prefix is /usr. */
    char prefix[PATH_MAX];
    snprintf(prefix, sizeof(prefix), "%s", library_dir);
    char *last = strrchr(prefix, '/');
    if (last != NULL && strcmp(last, "/lib") != 0 && strcmp(last, "/lib64") != 0) {
        *last = '\0';
        last = strrchr(prefix, '/');
    }
    if (last != NULL && (strcmp(last, "/lib") == 0 || strcmp(last, "/lib64") == 0)) {
        *last = '\0';
    }
    if (strcmp(prefix, "/usr") == 0) {
        snprintf(config_dir, sizeof(config_dir), "/etc/libibverbs.d");
    } else {
        snprintf(config_dir, sizeof(config_dir), "%s/etc/libibverbs.d", prefix);
    }

    DIR *config = opendir(config_dir);
    if (config == NULL) {
        fprintf(stderr, "no provider config directory %s (libibverbs would load no provider at all)\n", config_dir);
        return 1;
    }
    int driver_file_count = 0;
    struct dirent *entry;
    while ((entry = readdir(config)) != NULL) {
        size_t length = strlen(entry->d_name);
        if (length <= strlen(".driver") || strcmp(entry->d_name + length - strlen(".driver"), ".driver") != 0) {
            continue;
        }
        char driver_file[PATH_MAX];
        snprintf(driver_file, sizeof(driver_file), "%s/%s", config_dir, entry->d_name);
        if (read_driver_file(provider_dir, driver_file) > 0) {
            ++driver_file_count;
        }
    }
    closedir(config);

    if (driver_file_count == 0) {
        fprintf(stderr, "no .driver file in %s names a provider\n", config_dir);
        ++failure_count;
    }
    if (!is_mlx5_named || !is_efa_named) {
        fprintf(stderr, "the driver files do not name %s%s\n", is_mlx5_named ? "" : "mlx5 ", is_efa_named ? "" : "efa");
        ++failure_count;
    }
    if (failure_count != 0) {
        fprintf(stderr, "%d provider check(s) failed\n", failure_count);
        return 1;
    }
    printf("rdma-core: %d provider driver files, every named plugin loaded with RTLD_NOW\n", driver_file_count);
    return 0;
}
