/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for aws_c_io: pipe; calls aws_pipe_clean_up_write_end(), aws_mutex_lock(), aws_condition_variable_notify_all(), aws_mutex_unlock().
 */

#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/pipe.h>
#include <aws/common/allocator.h>
#include <aws/common/clock.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/common/task_scheduler.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

struct cleanup_ctx {
    struct aws_pipe_read_end *read_end;
    struct aws_pipe_write_end *write_end;
    struct aws_mutex mutex;
    struct aws_condition_variable cv;
    int read_rc;
    int write_rc;
    bool read_done;
    bool write_done;
};

static bool both_done_pred(void *user_data) {
    struct cleanup_ctx *c = user_data;
    return c->read_done && c->write_done;
}

static void cleanup_write_task(struct aws_task *task, void *arg, enum aws_task_status status) {
    (void)task;
    (void)status;
    struct cleanup_ctx *c = arg;
    int rc = aws_pipe_clean_up_write_end(c->write_end);
    aws_mutex_lock(&c->mutex);
    c->write_rc = rc;
    c->write_done = true;
    aws_condition_variable_notify_all(&c->cv);
    aws_mutex_unlock(&c->mutex);
}

static void cleanup_read_task(struct aws_task *task, void *arg, enum aws_task_status status) {
    (void)task;
    (void)status;
    struct cleanup_ctx *c = arg;
    int rc = aws_pipe_clean_up_read_end(c->read_end);
    aws_mutex_lock(&c->mutex);
    c->read_rc = rc;
    c->read_done = true;
    aws_condition_variable_notify_all(&c->cv);
    aws_mutex_unlock(&c->mutex);
}

int main(void) {
    struct aws_allocator *alloc = aws_default_allocator();
    assert(alloc != NULL);

    aws_io_library_init(alloc);

    struct aws_event_loop_group *elg = aws_event_loop_group_new_default(alloc, 1, NULL);
    assert(elg != NULL);

    struct aws_event_loop *loop = aws_event_loop_group_get_next_loop(elg);
    assert(loop != NULL);

    struct aws_pipe_read_end read_end;
    struct aws_pipe_write_end write_end;
    int rc = aws_pipe_init(&read_end, loop, &write_end, loop, alloc);
    assert(rc == AWS_OP_SUCCESS);

    /* Schedule cleanup tasks on the owning event-loop thread; the API
     * forbids synchronous clean-up from any other thread. */
    struct cleanup_ctx ctx = {
        .read_end = &read_end,
        .write_end = &write_end,
        .mutex = AWS_MUTEX_INIT,
        .cv = AWS_CONDITION_VARIABLE_INIT,
        .read_rc = AWS_OP_ERR,
        .write_rc = AWS_OP_ERR,
        .read_done = false,
        .write_done = false,
    };

    struct aws_task write_task;
    aws_task_init(&write_task, cleanup_write_task, &ctx, "cleanup_write");
    aws_event_loop_schedule_task_now(loop, &write_task);

    struct aws_task read_task;
    aws_task_init(&read_task, cleanup_read_task, &ctx, "cleanup_read");
    aws_event_loop_schedule_task_now(loop, &read_task);

    aws_mutex_lock(&ctx.mutex);
    int wait_rc = aws_condition_variable_wait_for_pred(
        &ctx.cv, &ctx.mutex,
        (int64_t)5 * AWS_TIMESTAMP_NANOS, both_done_pred, &ctx);
    aws_mutex_unlock(&ctx.mutex);
    assert(wait_rc == AWS_OP_SUCCESS);
    assert(ctx.write_rc == AWS_OP_SUCCESS);
    assert(ctx.read_rc == AWS_OP_SUCCESS);

    aws_event_loop_group_release(elg);

    aws_io_library_clean_up();

    printf("aws_c_io pipe test OK\n");
    return 0;
}
