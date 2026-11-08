/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for freetds: errhandler; calls dbinit(), dberrhandle(), dbmsghandle(), dbexit().
 */

/* The checks below use assert(), so keep it active even when the build passes -DNDEBUG. */
#undef NDEBUG

#include <sybfront.h>
#include <sybdb.h>
#include <assert.h>
#include <stdio.h>

static int err_handler_a(DBPROCESS *dbproc, int severity, int dberr,
                         int oserr, char *dberrstr, char *oserrstr) {
    (void)dbproc; (void)severity; (void)dberr;
    (void)oserr;  (void)dberrstr;  (void)oserrstr;
    return INT_CANCEL;
}

static int err_handler_b(DBPROCESS *dbproc, int severity, int dberr,
                         int oserr, char *dberrstr, char *oserrstr) {
    (void)dbproc; (void)severity; (void)dberr;
    (void)oserr;  (void)dberrstr;  (void)oserrstr;
    return INT_CANCEL;
}

static int msg_handler_a(DBPROCESS *dbproc, DBINT msgno, int msgstate,
                         int severity, char *msgtext, char *srvname,
                         char *procname, int line) {
    (void)dbproc;  (void)msgno;    (void)msgstate; (void)severity;
    (void)msgtext; (void)srvname;  (void)procname; (void)line;
    return 0;
}

static int msg_handler_b(DBPROCESS *dbproc, DBINT msgno, int msgstate,
                         int severity, char *msgtext, char *srvname,
                         char *procname, int line) {
    (void)dbproc;  (void)msgno;    (void)msgstate; (void)severity;
    (void)msgtext; (void)srvname;  (void)procname; (void)line;
    return 0;
}

int main(void) {
    assert(dbinit() == SUCCEED);

    EHANDLEFUNC prev_err = dberrhandle(err_handler_a);
    assert(prev_err == NULL);
    EHANDLEFUNC prev_err2 = dberrhandle(err_handler_b);
    assert(prev_err2 == err_handler_a);

    /* Message-handler registry: same contract. */
    MHANDLEFUNC prev_msg = dbmsghandle(msg_handler_a);
    assert(prev_msg == NULL);
    MHANDLEFUNC prev_msg2 = dbmsghandle(msg_handler_b);
    assert(prev_msg2 == msg_handler_a);

    /* Restore NULL handlers for cleanliness (and exercise the "deregister" path). */
    (void)dberrhandle(NULL);
    (void)dbmsghandle(NULL);

    dbexit();
    printf("freetds handler registry OK\n");
    return 0;
}
