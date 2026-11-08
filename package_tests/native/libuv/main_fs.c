/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026, Eugo
 *
 * Functional check for libuv: fs; calls uv_default_loop(), uv_fs_mkdtemp(), uv_fs_req_cleanup(), uv_fs_open().
 */

/* The checks below run through assert(); keep them live even when built with -DNDEBUG. */
#undef NDEBUG

#include <uv.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

int main(void) {
    uv_loop_t *loop = uv_default_loop();
    assert(loop != NULL);

    /* mkdtemp under /tmp */
    uv_fs_t mkdtemp_req;
    int r = uv_fs_mkdtemp(loop, &mkdtemp_req, "/tmp/demo_libuv_fs_XXXXXX", NULL);
    assert(r == 0);
    char dir[256];
    snprintf(dir, sizeof(dir), "%s", mkdtemp_req.path);
    uv_fs_req_cleanup(&mkdtemp_req);

    char path[512];
    snprintf(path, sizeof(path), "%s/payload.txt", dir);

    /* open O_CREAT | O_WRONLY */
    uv_fs_t open_req;
    int fd = uv_fs_open(loop, &open_req, path,
                       UV_FS_O_CREAT | UV_FS_O_WRONLY | UV_FS_O_TRUNC,
                       0600, NULL);
    assert(fd >= 0);
    uv_fs_req_cleanup(&open_req);

    /* write */
    const char payload[] = "demo-libuv-fs";
    uv_buf_t wbuf = uv_buf_init((char *)payload, sizeof(payload) - 1);
    uv_fs_t write_req;
    r = uv_fs_write(loop, &write_req, fd, &wbuf, 1, -1, NULL);
    assert(r == (int)(sizeof(payload) - 1));
    uv_fs_req_cleanup(&write_req);

    /* close */
    uv_fs_t close_req;
    r = uv_fs_close(loop, &close_req, fd, NULL);
    assert(r == 0);
    uv_fs_req_cleanup(&close_req);

    /* stat: size + regular file */
    uv_fs_t stat_req;
    r = uv_fs_stat(loop, &stat_req, path, NULL);
    assert(r == 0);
    assert(stat_req.statbuf.st_size == (uint64_t)(sizeof(payload) - 1));
    assert((stat_req.statbuf.st_mode & S_IFMT) == S_IFREG);
    uv_fs_req_cleanup(&stat_req);

    /* read it back */
    uv_fs_t open2_req;
    fd = uv_fs_open(loop, &open2_req, path, UV_FS_O_RDONLY, 0, NULL);
    assert(fd >= 0);
    uv_fs_req_cleanup(&open2_req);

    char readbuf[64] = {0};
    uv_buf_t rbuf = uv_buf_init(readbuf, sizeof(readbuf) - 1);
    uv_fs_t read_req;
    r = uv_fs_read(loop, &read_req, fd, &rbuf, 1, 0, NULL);
    assert(r == (int)(sizeof(payload) - 1));
    assert(memcmp(readbuf, payload, sizeof(payload) - 1) == 0);
    uv_fs_req_cleanup(&read_req);

    uv_fs_t close2_req;
    r = uv_fs_close(loop, &close2_req, fd, NULL);
    assert(r == 0);
    uv_fs_req_cleanup(&close2_req);

    /* unlink + rmdir */
    uv_fs_t unlink_req;
    r = uv_fs_unlink(loop, &unlink_req, path, NULL);
    assert(r == 0);
    uv_fs_req_cleanup(&unlink_req);

    uv_fs_t rmdir_req;
    r = uv_fs_rmdir(loop, &rmdir_req, dir, NULL);
    assert(r == 0);
    uv_fs_req_cleanup(&rmdir_req);

    assert(uv_loop_close(loop) == 0);

    printf("libuv fs test OK\n");
    return 0;
}
