#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "event_loop.h"
#include "generator.h"
#include "gather.h"
#include "utils.h"

#define SERVER_PORT (1337)

void client_handler(int sock)
{
    char buffer[256] = {0};
    while (1) {
        if (await_readable(sock) != AWAIT_OK) {
            break;
        }
        ssize_t nread = recv(sock, buffer, sizeof(buffer), 0);
        if (nread == 0) {
            break;
        }
        printf("read %ld bytes from fd %d\n", nread, sock);
        if (await_writable(sock) != AWAIT_OK) {
            break;
        }
        send(sock, buffer, nread, 0);
    }
    close(sock);
}

int accept_client(int sock)
{
    int client = -1;

    client = accept(sock, NULL, NULL);
    if (client == -1) {
        perror("Accepting client failed");
        goto error_ret;
    }

    int value = 4096;
    if (setsockopt(client, SOL_SOCKET, SO_RCVBUF, &value, sizeof(value)) != 0) {
        perror("Failed to set recv-buffer size");
    }

    if (fcntl(client, F_SETFD, O_NONBLOCK) != 0) {
        perror("Setting client socket as nonblocking failed");
        goto error_close;
    }

    if (create_task(generator_create(&client_handler, client)) != 0) {
        goto error_close;
    }

    return 0;

error_close:
    close(client);

error_ret:
    return -1;
}

void console_handler()
{
    char buffer[256] = {0};
    ssize_t nread = -1;

    while (1) {
        if (await_readable(STDIN_FILENO) != AWAIT_OK) {
            break;
        }

        nread = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);
        if (nread == -1) {
            perror("Failed to read from console");
            break;
        }

        if (memcmp("quit", buffer, MIN(4, nread)) == 0) {
            break;
        } else {
            printf("Unknown command: %s", buffer);
        }
    }

    loop_shutdown();
}

void run(int sock)
{
    while (1) {
        if (await_readable(sock) != AWAIT_OK) {
            break;
        }

        if (accept_client(sock) != 0) {
            break;
        }
    }

    loop_shutdown();
}

int create_server_socket(uint32_t ip, uint16_t port)
{
    int fd = -1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Failed to create server socket");
        goto error_ret;
    }

    int val = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        perror("Failed to set address re-use");
        goto error_close;
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(1337),
        .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        fprintf(stderr, "Failed to bind socket to %s:%d : %m\n", inet_ntoa(addr.sin_addr), htons(addr.sin_port));
        goto error_close;
    }

    if (listen(fd, 5) == -1) {
        perror("Failed to listen on server socket");
        goto error_close;
    }

    return fd;

error_close:
    close(fd);

error_ret:
    return fd;
}

void server()
{
    int fd = create_server_socket(INADDR_ANY, SERVER_PORT);
    if (fd == -1) {
        goto error_shutdown;
    }

    if (create_task(generator_create(&console_handler)) != 0) {
        goto error_close;
    }

    printf("server is listening on port %d\n", SERVER_PORT);
    run(fd);

error_close:
    close(fd);

error_shutdown:
    loop_shutdown();
}

int main(int argc, const char* argv[])
{
    generator_t* server_gen = generator_create(&server);
    if (server_gen == NULL) {
        fprintf(stderr, "server coroutine creation failed\n");
        return 1;
    }
    event_loop_run(server_gen);
    generator_destory(server_gen);
}
