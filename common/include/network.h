/*
 * AshEmu - WoW 2.4.3 Server Emulator
 * Copyright (C) 2025 AshEmu Team
 *
 * network.h - TCP server abstraction
 */

#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

/* Forward declarations */
typedef struct server server_t;
typedef struct client client_t;

/* Client handler callback - called for each new connection */
typedef void (*client_handler_t)(client_t *client, void *userdata);

/* Create TCP server on specified port */
server_t *server_create(int port, const char *name);

/* Free server resources */
void server_free(server_t *server);

/* Start listening (blocking call - use select internally) */
result_t server_run(server_t *server, client_handler_t handler, void *userdata);

/* Stop server (called from signal handler or another thread) */
void server_stop(server_t *server);

/* Get server port */
int server_get_port(const server_t *server);

/* Get server name */
const char *server_get_name(const server_t *server);

/* Client functions */

/* Get client socket */
socket_t client_get_socket(const client_t *client);

/* Get client remote address as string */
const char *client_get_address(const client_t *client);

/* Read data from client (blocking) */
ssize_t client_recv(client_t *client, uint8_t *buf, size_t len);

/* Read exact amount of data (blocking, returns OK or error) */
result_t client_recv_exact(client_t *client, uint8_t *buf, size_t len);

/* Send data to client */
ssize_t client_send(client_t *client, const uint8_t *buf, size_t len);

/* Send all data to client (blocking) */
result_t client_send_all(client_t *client, const uint8_t *buf, size_t len);

/* Close client connection */
void client_close(client_t *client);

/* Check if client is connected */
bool client_is_connected(const client_t *client);

/* Free client resources (called after handler returns or when closing) */
void client_free(client_t *client);

/* Create client from accepted socket (internal use) */
client_t *client_create(socket_t sock, struct sockaddr_in *addr);

#endif /* NETWORK_H */
