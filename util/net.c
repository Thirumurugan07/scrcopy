#include "net.h"
#include "log.h"

bool net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(1, 1), &wsa);
    if (res) {
        LOGE("WSAStartup failed with error %d", res);
        return false;
    }
#endif
    return true;
}

void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static inline sc_socket wrap(sc_raw_socket sock) {
    sc_socket s = malloc(sizeof(*s));
    if (!s) {
        LOG_OOM();
        return NULL;
    }
    s->socket = sock;
    return s;
}

static inline sc_raw_socket unwrap(sc_socket socket) {
    return socket->socket;
}

bool net_connect(sc_socket socket, uint32_t addr, uint16_t port) {
    sc_raw_socket raw_sock = unwrap(socket);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr);
    sin.sin_port = htons(port);

    if (connect(raw_sock, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        perror("connect");
        return false;
    }

    return true;
}

bool net_listen(sc_socket server_socket, uint32_t addr, uint16_t port, int backlog) {
    sc_raw_socket raw_sock = unwrap(server_socket);

    int reuse = 1;
    if (setsockopt(raw_sock, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuse,
        sizeof(reuse)) == -1) {
        perror("setsockopt(SO_REUSEADDR)");
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(addr); // htonl() harmless on INADDR_ANY
    sin.sin_port = htons(port);

    if (bind(raw_sock, (struct sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) {
        perror("bind");
        return false;
    }

    if (listen(raw_sock, backlog) == SOCKET_ERROR) {
        perror("listen");
        return false;
    }

    return true;
}

sc_socket net_accept(sc_socket server_socket) {
    sc_raw_socket raw_server_socket = unwrap(server_socket);

    struct sockaddr_in csin;
    socklen_t sinsize = sizeof(csin);

    sc_raw_socket raw_sock = accept(raw_server_socket, (struct sockaddr*)&csin, &sinsize);
    if (raw_sock == SC_RAW_SOCKET_NONE) {
        perror("accept");
        return NULL;
    }

    return wrap(raw_sock);
}

ssize_t net_recv(sc_socket socket, void* buf, size_t len) {
    sc_raw_socket raw_sock = unwrap(socket);
    return recv(raw_sock, buf, len, 0);
}

ssize_t net_recv_all(sc_socket socket, void* buf, size_t len) {
    sc_raw_socket raw_sock = unwrap(socket);

    return recv(raw_sock, buf, len, MSG_WAITALL);
}

ssize_t net_send(sc_socket socket, const void* buf, size_t len) {
    sc_raw_socket raw_sock = unwrap(socket);
    return send(raw_sock, buf, len, 0);
}

ssize_t net_send_all(sc_socket socket, const void* buf, size_t len) {
    size_t copied = 0;
    while (len > 0) {
        int w = net_send(socket, buf, len);
        if (w == -1) {
            return copied ? (int)copied : -1;
        }
        len -= w;
        buf = (char*)buf + w;
        copied += w;
    }
    return copied;
}
sc_socket
net_socket(void) {
    sc_raw_socket raw_sock;

    raw_sock = socket(AF_INET, SOCK_STREAM, 0);

    sc_socket sock = wrap(raw_sock);
    if (sock == SC_SOCKET_NONE) {
        perror("socket");
    }
    return sock;
}



bool net_interrupt(sc_socket socket) {
    sc_raw_socket raw_sock = unwrap(socket);

#ifdef _WIN32
    if (shutdown(raw_sock, SD_BOTH) == SOCKET_ERROR) {
        perror("shutdown");
        return false;
    }
#else
    if (shutdown(raw_sock, SHUT_RDWR) == SOCKET_ERROR) {
        perror("shutdown");
        return false;
    }
#endif

    return true;
}

bool net_close(sc_socket socket) {
    sc_raw_socket raw_sock = unwrap(socket);

    if (closesocket(raw_sock) == SOCKET_ERROR) {
        perror("closesocket");
        return false;
    }

    free(socket);

    return true;
}

bool net_parse_ipv4(const char* s, uint32_t* ipv4) {
    struct in_addr addr;
    if (!inet_pton(AF_INET, s, &addr)) {
        LOGE("Invalid IPv4 address: %s", s);
        return false;
    }

    *ipv4 = ntohl(addr.s_addr);
    return true;
}
