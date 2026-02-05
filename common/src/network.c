/*
 * AshEmu - WoW 1.12.1 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * network.c - TCP server implementation
 */

#include "network.h"

/* Server structure */
struct server {
    socket_t sock;
    int port;
    char name[64];
    volatile bool running;
};

/* Client structure */
struct client {
    socket_t sock;
    char address[64];
    bool connected;
};

/* Create TCP server */
server_t *server_create(int port, const char *name) {
    server_t *server = ALLOC(server_t);
    if (!server) return NULL;

    server->port = port;
    safe_strncpy(server->name, name, sizeof(server->name));
    server->running = false;
    server->sock = INVALID_SOCKET;

    return server;
}

/* Free server resources */
void server_free(server_t *server) {
    if (!server) return;
    if (server->sock != INVALID_SOCKET) {
        socket_close(server->sock);
    }
    free(server);
}

/* Start listening */
result_t server_run(server_t *server, client_handler_t handler, void *userdata) {
    /* Create socket */
    server->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server->sock == INVALID_SOCKET) {
        LOG_ERROR(server->name, "Failed to create socket: %d", socket_errno);
        return ERR_NETWORK;
    }

    /* Set reuse address */
    int opt = 1;
    setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    /* Bind */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)server->port);

    if (bind(server->sock, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        LOG_ERROR(server->name, "Failed to bind to port %d: %d", server->port, socket_errno);
        socket_close(server->sock);
        server->sock = INVALID_SOCKET;
        return ERR_NETWORK;
    }

    /* Listen */
    if (listen(server->sock, SOMAXCONN) == SOCKET_ERROR) {
        LOG_ERROR(server->name, "Failed to listen: %d", socket_errno);
        socket_close(server->sock);
        server->sock = INVALID_SOCKET;
        return ERR_NETWORK;
    }

    LOG_INFO(server->name, "Listening on port %d", server->port);
    server->running = true;

    /* Accept loop using select */
    while (server->running) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server->sock, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int result = select((int)server->sock + 1, &read_fds, NULL, NULL, &timeout);
        if (result == SOCKET_ERROR) {
            if (!server->running) break;
            LOG_ERROR(server->name, "Select error: %d", socket_errno);
            continue;
        }

        if (result == 0) {
            /* Timeout, check running flag */
            continue;
        }

        if (FD_ISSET(server->sock, &read_fds)) {
            struct sockaddr_in client_addr;
            int addr_len = sizeof(client_addr);
            socket_t client_sock = accept(server->sock, (struct sockaddr*)&client_addr, &addr_len);

            if (client_sock == INVALID_SOCKET) {
                if (!server->running) break;
                LOG_ERROR(server->name, "Accept error: %d", socket_errno);
                continue;
            }

            /* Create client and call handler */
            client_t *client = client_create(client_sock, &client_addr);
            if (client && handler) {
                handler(client, userdata);
            }
        }
    }

    socket_close(server->sock);
    server->sock = INVALID_SOCKET;
    return OK;
}

/* Stop server */
void server_stop(server_t *server) {
    server->running = false;
}

/* Get server port */
int server_get_port(const server_t *server) {
    return server->port;
}

/* Get server name */
const char *server_get_name(const server_t *server) {
    return server->name;
}

/* Create client from accepted socket */
client_t *client_create(socket_t sock, struct sockaddr_in *addr) {
    client_t *client = ALLOC(client_t);
    if (!client) {
        socket_close(sock);
        return NULL;
    }

    client->sock = sock;
    client->connected = true;

    /* Format address string */
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    snprintf(client->address, sizeof(client->address), "%s:%d", ip, ntohs(addr->sin_port));

    return client;
}

/* Get client socket */
socket_t client_get_socket(const client_t *client) {
    return client->sock;
}

/* Get client address */
const char *client_get_address(const client_t *client) {
    return client->address;
}

/* Read data from client */
ssize_t client_recv(client_t *client, uint8_t *buf, size_t len) {
    if (!client->connected) return -1;

    ssize_t result = recv(client->sock, (char*)buf, (int)len, 0);
    if (result <= 0) {
        client->connected = false;
    }
    return result;
}

/* Read exact amount of data */
result_t client_recv_exact(client_t *client, uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t result = client_recv(client, buf + total, len - total);
        if (result <= 0) {
            return ERR_DISCONNECTED;
        }
        total += result;
    }
    return OK;
}

/* Send data to client */
ssize_t client_send(client_t *client, const uint8_t *buf, size_t len) {
    if (!client->connected) return -1;

    ssize_t result = send(client->sock, (const char*)buf, (int)len, 0);
    if (result < 0) {
        client->connected = false;
    }
    return result;
}

/* Send all data to client */
result_t client_send_all(client_t *client, const uint8_t *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t result = client_send(client, buf + total, len - total);
        if (result < 0) {
            return ERR_DISCONNECTED;
        }
        total += result;
    }
    return OK;
}

/* Close client connection */
void client_close(client_t *client) {
    if (client->connected) {
        socket_close(client->sock);
        client->sock = INVALID_SOCKET;
        client->connected = false;
    }
}

/* Check if client is connected */
bool client_is_connected(const client_t *client) {
    return client->connected;
}

/* Free client resources */
void client_free(client_t *client) {
    if (!client) return;
    client_close(client);
    free(client);
}
