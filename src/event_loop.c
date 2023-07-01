#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "event_loop.h"
#include "list.h"
#include "utils.h"

#define MAX_FDS (1024)

static struct loop_s {
    int max_fd;
    fd_set readers;
    list_t read_queues[MAX_FDS];
    fd_set writers;
    list_t write_queues[MAX_FDS];
    list_t scheduled;
    list_t tasks;
    int stopped;
} loop;

void execute_scheduled();

void handle_readers(fd_set* read_ready);
void handle_writers(fd_set* read_ready);

void cancel_gen_list(list_t* gen_list, int do_destory)
{
    while (!list_is_empty(gen_list)) {
        generator_t* gen = (generator_t*)list_pop(gen_list);
        next(gen, AWAIT_CANCELLED);
        if (do_destory) {
            generator_destory(gen);
        }
    }
}

/**
 * During execution of scheduled calls, some more calls may be added.
 * This will run until all coroutines have wither finished, or blocked
 * on some other await primitives.
 */
static void exhuast_scheduled()
{
    while (!list_is_empty(&loop.scheduled) && !loop.stopped) {
        execute_scheduled();
    }
}

static void loop_setup()
{
    loop.max_fd = 0;
    for (size_t i = 0; i < ARRAY_SIZE(loop.read_queues); i++) {
        list_init(&loop.read_queues[i]);
    }
    FD_ZERO(&loop.readers);
    for (size_t i = 0; i < ARRAY_SIZE(loop.write_queues); i++) {
        list_init(&loop.write_queues[i]);
    }
    FD_ZERO(&loop.writers);
    list_init(&loop.scheduled);
    list_init(&loop.tasks);
    loop.stopped = 0;
}

static int event_loop_select()
{
    fd_set readers_copy;
    fd_set writers_copy;
    memcpy(&readers_copy, &loop.readers, sizeof(loop.readers));  // preserve the existing state
    memcpy(&writers_copy, &loop.writers, sizeof(loop.writers));  // preserve the existing state

    int available = select(loop.max_fd, &readers_copy, &writers_copy, NULL, NULL);
    if (available == -1) {
        perror("Event loop select error");
        return -1;
    }

    handle_readers(&readers_copy);

    handle_writers(&writers_copy);

    return 0;
}

void loop_teardown()
{
    for (size_t i = 0; i < ARRAY_SIZE(loop.read_queues); ++i) {
        cancel_gen_list(&loop.read_queues[i], 0);
    }

    cancel_gen_list(&loop.scheduled, 0);

    cancel_gen_list(&loop.tasks, 1);
}

void event_loop_run(generator_t* gen)
{
    loop_setup();

    next(gen);

    while (gen->status != GEN_STATUS_DONE && !loop.stopped) {
        exhuast_scheduled();

        if (event_loop_select() != 0) {
            loop.stopped = 1;
        }
    }

    loop_teardown();
}

static void handle_event(fd_set* req_event_set, fd_set* cur_event_set, list_t* event_queues)
{
    for (int i = 0; i < loop.max_fd && !loop.stopped; i++) {
        list_t* registered = &event_queues[i];
        if (!FD_ISSET(i, cur_event_set)) {
            continue;
        }
        generator_t* gen = (generator_t*)list_pop(registered);
        if (gen != NULL) {
            next(gen, AWAIT_OK);
        }
        if (list_is_empty(registered)) {
            FD_CLR(i, req_event_set);
        }
    }
}

void handle_writers(fd_set* write_ready)
{
    handle_event(&loop.writers, write_ready, loop.write_queues);
}

void handle_readers(fd_set* read_ready)
{
    handle_event(&loop.readers, read_ready, loop.read_queues);
    /* for (int i = 0; i < loop.max_fd && !loop.stopped; i++) { */
    /*     list_t* registered = &loop.read_queues[i]; */
    /*     if (!FD_ISSET(i, read_ready)) { */
    /*         continue; */
    /*     } */
    /*     generator_t* gen = (generator_t*)list_pop(registered); */
    /*     if (gen != NULL) { */
    /*         next(gen, AWAIT_OK); */
    /*     } */
    /*     if (list_is_empty(registered)) { */
    /*         FD_CLR(i, &loop.readers); */
    /*     } */
    /* } */
}

static await_status_t await_event(int fd, fd_set* event_set, list_t* event_queue)
{
    loop.max_fd = MAX(loop.max_fd, fd + 1);
    FD_SET(fd, event_set);
    if (list_add(event_queue, current) != 0) {
        return AWAIT_ERR;
    }
    return yield(AWAIT_OK);
}

static void cancel_await_event(int fd, fd_set* event_set, list_t* event_queue)
{
    list_remove(event_queue, current);
    if (list_is_empty(event_queue)) {
        FD_CLR(fd, event_set);
    }
}

await_status_t await_readable(int fd)
{
    return await_event(fd, &loop.readers, &loop.read_queues[fd]);
}

void cancel_await_readable(int fd)
{
    cancel_await_event(fd, &loop.readers, &loop.read_queues[fd]);
}

await_status_t await_writable(int fd)
{
    return await_event(fd, &loop.writers, &loop.write_queues[fd]);
}

void cancel_await_writable(int fd)
{
    cancel_await_event(fd, &loop.writers, &loop.write_queues[fd]);
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

    while (!list_is_empty(&curr_scheduled) && !loop.stopped) {
        generator_t* gen = (generator_t*)list_pop(&curr_scheduled);
        next(gen, AWAIT_OK);
    }
}

int create_task_from(generator_t* gen)
{
    if (list_add(&loop.tasks, gen) != 0) {
        return -1;
    }
    if (call_soon(gen) != 0) {
        list_remove(&loop.tasks, gen);
        return -1;
    }
    return 0;
}

int create_task(void* func, ...)
{
    va_list ap;
    va_start(ap, func);
    void* arg = va_arg(ap, void*);
    va_end(ap);

    generator_t* task = generator_create(func, arg);
    if (task == NULL) {
        return -1;
    }

    return create_task_from(task);
}

void loop_shutdown(void)
{
    loop.stopped = 1;
}
