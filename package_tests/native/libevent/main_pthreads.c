/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libevent: pthreads; calls evthread_use_pthreads(), event_base_new(), event_base_loop(), event_base_loopbreak(), event_base_got_break(), event_base_free().
 */

#include <event2/event.h>
#include <event2/thread.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

/* Exercises the libevent_pthreads side library: with pthread lock callbacks
 * registered, a base must be safe to drive from ANOTHER thread.
 *
 * evthread_use_pthreads() must be called BEFORE any event_base is constructed;
 * a base built afterwards gets locks and a notification fd. The check is
 * cross-thread: the main thread blocks in the event loop, and a second
 * pthread calls event_base_loopbreak() on it. With threading set up, the
 * break takes the base lock and writes the notification fd, which wakes the
 * loop at once. Without it (no lock callbacks, or a libevent_pthreads that
 * registers nothing) the break only sets a flag, the loop stays asleep, and
 * the 5 s fallback timer below fires instead: that is the failure this probe
 * reports.
 *
 * Ordering: the second thread is started from a zero-delay timer callback, so
 * the loop is already running when it calls event_base_loopbreak()
 * (event_base_loop clears a break requested before it starts). */

#define FALLBACK_TIMEOUT_S 5

static int g_fallback_fired = 0;

struct probe_ctx {
    struct event_base *base;
    pthread_t thread;
    int thread_started;
};

static void *breaker(void *arg) {
    struct event_base *base = arg;
    usleep(100 * 1000); /* let the loop go back to sleep in the backend */
    event_base_loopbreak(base);
    return NULL;
}

static void on_start(evutil_socket_t fd, short what, void *arg) {
    (void)fd;
    (void)what;
    struct probe_ctx *ctx = arg;
    if (pthread_create(&ctx->thread, NULL, breaker, ctx->base) != 0) {
        fprintf(stderr, "FAIL: pthread_create failed\n");
        _exit(1);
    }
    ctx->thread_started = 1;
}

static void on_fallback(evutil_socket_t fd, short what, void *arg) {
    (void)fd;
    (void)what;
    g_fallback_fired = 1;
    event_base_loopbreak((struct event_base *)arg);
}

int main(void) {
    /* Register pthread lock callbacks first, before any base exists. */
    if (evthread_use_pthreads() != 0) {
        fprintf(stderr, "FAIL: evthread_use_pthreads() did not return 0\n");
        return 1;
    }

    struct event_base *base = event_base_new();
    if (base == NULL) {
        fprintf(stderr, "FAIL: event_base_new() returned NULL\n");
        return 1;
    }

    struct probe_ctx ctx = {base, 0, 0};
    struct event *start = evtimer_new(base, on_start, &ctx);
    struct event *fallback = evtimer_new(base, on_fallback, base);
    if (start == NULL || fallback == NULL) {
        fprintf(stderr, "FAIL: evtimer_new() returned NULL\n");
        return 1;
    }
    struct timeval now = {0, 0};
    struct timeval later = {FALLBACK_TIMEOUT_S, 0};
    evtimer_add(start, &now);
    evtimer_add(fallback, &later);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    int rc = event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    if (ctx.thread_started) {
        pthread_join(ctx.thread, NULL);
    }
    double waited = (double)(t1.tv_sec - t0.tv_sec) + (double)(t1.tv_nsec - t0.tv_nsec) / 1e9;

    if (rc != 0) {
        fprintf(stderr, "FAIL: event_base_loop returned %d\n", rc);
        return 1;
    }
    if (g_fallback_fired || !event_base_got_break(base)) {
        fprintf(stderr,
                "FAIL: event_base_loopbreak() from a second thread did not wake the loop "
                "(woke after %.2f s on the %d s fallback timer): the base has no working "
                "pthread locking/notification\n",
                waited, FALLBACK_TIMEOUT_S);
        return 1;
    }

    event_free(fallback);
    event_free(start);
    event_base_free(base);
    printf("libevent pthreads test OK (cross-thread loopbreak woke the loop in %.3f s)\n", waited);
    return 0;
}
