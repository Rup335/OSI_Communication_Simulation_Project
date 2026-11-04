#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osi_common.h"
#include "layers.h"

int main(int argc, char *argv[]) {
    const char *message = "Hello OSI Model";
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;

    if (argc >= 2) {
        message = argv[1];
    }
    if (argc >= 3) {
        host = argv[2];
    }
    if (argc >= 4) {
        port = atoi(argv[3]);
    }

    printf("====================================================\n");
    printf("   OSI 7-LAYER SIMULATION - C CLIENT APPLICATION\n");
    printf("====================================================\n");
    printf("Target Server : %s:%d\n", host, port);
    printf("User Message  : \"%s\"\n\n", message);

    if (osi_socket_init() != 0) {
        fprintf(stderr, "[Client Error] Failed to initialize socket subsystem.\n");
        return 1;
    }

    SOCKET client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == INVALID_SOCKET) {
        fprintf(stderr, "[Client Error] Failed to create socket.\n");
        osi_socket_cleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);
    server_addr.sin_addr.s_addr = inet_addr(host);

    printf("[net:osi] Connecting to server at %s:%d...\n", host, port);
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "[net:osi:error] Failed to connect to server at %s:%d! Ensure server is running.\n", host, port);
        SOCKET_CLOSE(client_sock);
        osi_socket_cleanup();
        return 1;
    }
    printf("[net:osi] [ OK ] Real TCP socket connection established!\n");

    // Perform OSI 7-Layer Encapsulation
    char final_frame[MAX_BUFFER];
    if (osi_client_encapsulate(message, final_frame, sizeof(final_frame)) != 0) {
        fprintf(stderr, "[Client Error] Encapsulation failed.\n");
        SOCKET_CLOSE(client_sock);
        osi_socket_cleanup();
        return 1;
    }

    // Physical layer transmission over real socket
    if (physical_send(final_frame, client_sock) != 0) {
        fprintf(stderr, "[Client Error] Transmission over socket failed.\n");
        SOCKET_CLOSE(client_sock);
        osi_socket_cleanup();
        return 1;
    }

    // Receive acknowledgment from server
    char ack[64];
    uint32_t ack_len = 0;
    if (recv_all(client_sock, &ack_len, sizeof(ack_len)) == 0) {
        ack_len = ntohl(ack_len);
        if (ack_len < sizeof(ack) && recv_all(client_sock, ack, ack_len) == 0) {
            ack[ack_len] = '\0';
            printf("[net:osi] Server Response: %s\n", ack);
        }
    }

    printf("\n=== [CLIENT] ALL 7 LAYERS PROCESSED SUCCESSFULLY ===\n");
    SOCKET_CLOSE(client_sock);
    osi_socket_cleanup();
    return 0;
}
