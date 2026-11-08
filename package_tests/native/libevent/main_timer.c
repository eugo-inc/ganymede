/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libevent: timer; calls event_base_loopexit(), event_base_new(), evtimer_new(), evtimer_add().
 */

#include <event2/event.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Exercises libevent's timer-event + dispatch loop: distinct from main.c which
 * only creates+frees an event_base. Failure here means EV_TIMEOUT scheduling
 * or the event-loop dispatch is broken. */

static int callback_fired = 0;

static void on_timer(evutil_socket_t fd, short events, void *arg) {
    (void)fd;
    assert(events & EV_TIMEOUT);
    callback_fired = 1;
    struct event_base *base = (struct event_base *)arg;
    /* Stop the loop so event_base_dispatch returns. */
    event_base_loopexit(base, NULL);
}

int main(void) {
    struct event_base *base = event_base_new();
    assert(base != NULL);

    struct timeval tv = {0, 1000};  /* 1ms */
    struct event *ev = evtimer_new(base, on_timer, base);
    assert(ev != NULL);

    int rc = evtimer_add(ev, &tv);
    assert(rc == 0);

    rc = event_base_dispatch(base);
    assert(rc >= 0);
    assert(callback_fired == 1);

    event_free(ev);
    event_base_free(base);
    printf("libevent timer test OK\n");
    return 0;
}
