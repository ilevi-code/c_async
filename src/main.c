#include "generator.h"
#include "tests.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "event_loop.h"
#include "future.h"
#include "tasks.h"
#include "utils.h"

void client_handler(int sock)
{
    char buffer[256] = {0};
    while (1) {
        if (await_readable(sock) != AWAIT_OK) {
            break;
        }
        ssize_t nread = read(sock, buffer, sizeof(buffer));
        printf("read %ld bytes from fd %d\n", nread, sock);
    }
    close(sock);
}

int accept_client(int sock)
{
    int client = accept(sock, NULL, NULL);
    if (client == -1) {
        perror("Accepting client failed");
        return -1;
    }
    generator_t* handler = generator_create(&client_handler, client);
    if (handler == NULL) {
        close(client);
        return -1;
    }
    if (create_task(handler) != 0) {
        close(client);
        generator_destory(handler);
        return -1;
    }
    return 0;
}

#include <string.h>
int handle_input()
{
    char buffer[256] = {0};
    ssize_t nread = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
    if (nread == -1) {
        perror("Failed to read from console");
        return 1;
    }
    if (memcmp("quit", buffer, MIN(4, nread)) == 0) {
        return 1;
    }
    printf("Unknown command: %s", buffer);
    return 0;
}

void run(int sock)
{
    size_t ready_idx;
    int done = 0;

    while (!done) {
        future_descriptor_t descriptors[] = {
            {await_readable, cancel_await_readable, .val = sock},
            {await_readable, cancel_await_readable, .val = STDIN_FILENO},
        };

        ready_idx = desc_gather(descriptors, ARRAY_SIZE(descriptors));
        switch (ready_idx) {
        case 0:
            if (accept_client(sock) != 0) {
                done = 1;
            } else {
                printf("Accepted a client!\n");
            }
            break;
        case 1:
            done = handle_input();
            break;
        default:
            fprintf(stderr, "Error waiting for console/socket\n");
            break;
        }
    }
}

void server()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        return;
    }

    int val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        perror("setsockopt");
        close(fd);
        return;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(1337),
        .sin_addr = {.s_addr = INADDR_ANY},
    };

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(fd);
        return;
    }

    if (listen(fd, 5) == -1) {
        perror("listen");
        close(fd);
        return;
    }

    printf("server is up\n");
    run(fd);

    close(fd);
    return;
}

int main(int argc, const char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "--run-tests") == 0) {
        run_tests();
        return 0;
    }

    generator_t* server_gen = generator_create(&server);
    if (server_gen == NULL) {
        fprintf(stderr, "server coroutine creation failed\n");
        return 1;
    }
    event_loop_run(server_gen);
    generator_destory(server_gen);
}
