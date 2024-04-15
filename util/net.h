#ifndef NET_H
#define NET_H

#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#include <BaseTsd.h>
typedef SSIZE_T ssize_t;

typedef SOCKET sc_raw_socket;
#define SC_RAW_SOCKET_NONE INVALID_SOCKET


typedef struct sc_socket_windows {
    SOCKET socket;
    bool closed;
} sc_socket_windows;

typedef sc_socket_windows* sc_socket;

#define SC_SOCKET_NONE NULL

bool net_init(void);

void net_cleanup(void);

sc_socket net_socket(void);

bool net_connect(sc_socket socket, uint32_t addr, uint16_t port);

bool net_listen(sc_socket server_socket, uint32_t addr, uint16_t port, int backlog);

sc_socket net_accept(sc_socket server_socket);

ssize_t net_recv(sc_socket socket, void* buf, size_t len);

ssize_t net_recv_all(sc_socket socket, void* buf, size_t len);

ssize_t net_send(sc_socket socket, const void* buf, size_t len);

ssize_t net_send_all(sc_socket socket, const void* buf, size_t len);

bool net_interrupt(sc_socket socket);

bool net_close(sc_socket socket);

bool net_parse_ipv4(const char* s, uint32_t* ipv4);

#endif
