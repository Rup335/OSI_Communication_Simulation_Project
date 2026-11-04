#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osi_common.h"
#include "layers.h"

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    int run_once = 0;

    if (argc >= 2) {
        port = atoi(argv[1]);
    }
    if (argc >= 3 && strcmp(argv[2], "--once") == 0) {
        run_once = 1;
    }

    printf("====================================================\n");
    printf("   OSI 7-LAYER SIMULATION - C SERVER APPLICATION\n");
    printf("====================================================\n");
    printf("Listening on port: %d (TCP Socket)\n\n", port);

    if (osi_socket_init() != 0) {
        fprintf(stderr, "[Server Error] Failed to initialize socket subsystem.\n");
        return 1;
    }

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        fprintf(stderr, "[Server Error] Failed to create socket.\n");
        osi_socket_cleanup();
        return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
#else
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[Server Error] Failed to bind to port %d.\n", port);
        SOCKET_CLOSE(server_sock);
        osi_socket_cleanup();
        return 1;
    }

    if (listen(server_sock, 5) == SOCKET_ERROR) {
        fprintf(stderr, "[Server Error] Failed to listen on socket.\n");
        SOCKET_CLOSE(server_sock);
        osi_socket_cleanup();
        return 1;
    }

    printf("[net:osi] [ OK ] Socket listening on 0.0.0.0:%d... Waiting for client.\n\n", port);

    do {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        SOCKET client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock == INVALID_SOCKET) {
            fprintf(stderr, "[Server Error] Failed to accept connection.\n");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("\n>>> [CONNECTION ACCEPTED] Client: %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        char frame[MAX_BUFFER];
        char recovered[MAX_BUFFER];

        // 1. Layer 1: Physical receive from real socket
        if (physical_recv(client_sock, frame, sizeof(frame)) == 0) {
            // 2. Full Decapsulation (Layers 2 through 7)
            if (osi_server_decapsulate(frame, recovered, sizeof(recovered)) == 0) {
                // Send acknowledgment back
                const char *ack = "ACK[STATUS=DELIVERED,PROTOCOL=OSI_7_LAYER]";
                uint32_t ack_len = (uint32_t)strlen(ack);
                uint32_t net_ack = htonl(ack_len);
                send_all(client_sock, &net_ack, sizeof(net_ack));
                send_all(client_sock, ack, ack_len);
            } else {
                fprintf(stderr, "[Server Error] Decapsulation failed.\n");
            }
        } else {
            fprintf(stderr, "[Server Error] Failed to receive frame from socket.\n");
        }

        SOCKET_CLOSE(client_sock);
        printf(">>> Connection closed with %s. Ready for next packet.\n", client_ip);

    } while (!run_once);

    SOCKET_CLOSE(server_sock);
    osi_socket_cleanup();
    return 0;
}
