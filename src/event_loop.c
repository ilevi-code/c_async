#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "event_loop.h"
#include "list.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_FDS (1024)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

static struct loop_s {
    fd_set readers;
    int max_fd;
    list_t registered[MAX_FDS];
    list_t scheduled;
} loop;

void execute_scheduled();

void handle_readers(fd_set* read_ready);

void event_loop_run(generator_t* gen)
{
    loop.max_fd = 0;
    for (size_t i = 0; i < ARRAY_SIZE(loop.registered); i++) {
        list_init(&loop.registered[i]);
    }
    list_init(&loop.scheduled);
    FD_ZERO(&loop.readers);
    fd_set readers_copy;

    next(gen);

    while (gen->status != GEN_STATUS_DONE) {
        memcpy(&readers_copy, &loop.readers, sizeof(loop.readers));  // preserve the existing state
        int available = select(loop.max_fd, &readers_copy, NULL, NULL, NULL);
        if (available == -1) {
            perror("Event loop select error");
            return;
        }

        handle_readers(&readers_copy);

        while (!list_is_empty(&loop.scheduled)) {
            execute_scheduled();
        }
    }
}

void handle_readers(fd_set* read_ready)
{
    for (int i = 0; i < loop.max_fd; i++) {
        list_t* registered = &loop.registered[i];
        if (!FD_ISSET(i, read_ready)) {
            continue;
        }
        generator_t* gen = (generator_t*)list_pop(registered);
        if (gen != NULL) {
            next(gen, AWAIT_OK);
        }
        if (list_is_empty(registered)) {
            FD_CLR(i, &loop.readers);
        }
    }
}

int await_readable(int fd)
{
    loop.max_fd = MAX(loop.max_fd, fd + 1);
    FD_SET(fd, &loop.readers);
    if (list_add(&loop.registered[fd], current) != 0) {
        return AWAIT_ERR;
    }
    return yield(AWAIT_OK);
}

void cancel_await_readable(int fd)
{
    list_t* reader_list = &loop.registered[fd];
    list_remove(reader_list, current);
    if (list_is_empty(reader_list)) {
        FD_CLR(fd, &loop.readers);
    }
}

int call_soon(generator_t* gen)
{
    if (!list_contains(&loop.scheduled, gen)) {
        return list_add(&loop.scheduled, gen);
    }
    return 0;
}

void execute_scheduled()
{
    list_t curr_scheduled;
    // prevent modification when resuming context in the scheduled coroutine.
    list_steal(&curr_scheduled, &loop.scheduled);

    while (!list_is_empty(&curr_scheduled)) {
        generator_t* gen = (generator_t*)list_pop(&curr_scheduled);
        next(gen, AWAIT_OK);
    }
}
