/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo, Inc.
 *
 * Functional check for perl: embed; calls newXS(), perl_alloc(), perl_construct(), perl_parse().
 */

#include <EXTERN.h>
#include <perl.h>

EXTERN_C void boot_DynaLoader(pTHX_ CV *cv);

static void xs_init(pTHX) {
    newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);
}

int main(int argc, char **argv, char **env) {
    char *embedding[] = {"", "-e",
                         "use strict; use FindBin; use Compress::Raw::Zlib;"
                         " exit(Compress::Raw::Zlib::crc32('perl') == 1567244364 ? 0 : 3);",
                         NULL};
    PERL_SYS_INIT3(&argc, &argv, &env);
    PerlInterpreter *my_perl = perl_alloc();
    if (my_perl == NULL) {
        return 10;
    }
    perl_construct(my_perl);
    PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
    if (perl_parse(my_perl, xs_init, 3, embedding, NULL) != 0) {
        return 11;
    }
    int status = perl_run(my_perl);
    perl_destruct(my_perl);
    perl_free(my_perl);
    PERL_SYS_TERM();
    return status;
}
