#ifndef OSI_COMMON_H
#define OSI_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #define SOCKET_CLOSE(s) closesocket(s)
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
    #define SOCKET_CLOSE(s) close(s)
#endif

#define DEFAULT_PORT 8080
#define DEFAULT_HOST "127.0.0.1"
#define MAX_BUFFER   4096

// Cross-platform socket initialization
static inline int osi_socket_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    int res = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (res != 0) {
        fprintf(stderr, "[Socket Init Error] WSAStartup failed: %d\n", res);
        return -1;
    }
#endif
    return 0;
}

// Cross-platform socket cleanup
static inline void osi_socket_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Ensure all bytes are sent across TCP stream
static inline int send_all(SOCKET sock, const void *buffer, size_t length) {
    const char *ptr = (const char *)buffer;
    size_t total_sent = 0;
    while (total_sent < length) {
        int sent = send(sock, ptr + total_sent, (int)(length - total_sent), 0);
        if (sent <= 0) {
            return -1;
        }
        total_sent += (size_t)sent;
    }
    return 0;
}

// Ensure all bytes are received from TCP stream
static inline int recv_all(SOCKET sock, void *buffer, size_t length) {
    char *ptr = (char *)buffer;
    size_t total_received = 0;
    while (total_received < length) {
        int received = recv(sock, ptr + total_received, (int)(length - total_received), 0);
        if (received <= 0) {
            return -1;
        }
        total_received += (size_t)received;
    }
    return 0;
}

// Simple 8-bit checksum for Layer 2 Frame Check Sequence (FCS)
static inline uint8_t compute_checksum(const char *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum ^= (uint8_t)data[i];
    }
    return sum;
}

#endif // OSI_COMMON_H
