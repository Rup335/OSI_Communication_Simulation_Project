#include <stdio.h>
#include <string.h>
#include "../src/osi_common.h"
#include "../src/layers.h"

int main(int argc, char *argv[]) {
    const char *msg = "Hello OSI Model";
    if (argc >= 2) {
        msg = argv[1];
    }

    char frame[MAX_BUFFER];
    char recovered[MAX_BUFFER];

    printf("====================================================\n");
    printf("   OSI 7-LAYER SIMULATION - LINUX/UBUNTU STACK TEST\n");
    printf("====================================================\n");

    // 1. Client Encapsulation
    if (osi_client_encapsulate(msg, frame, sizeof(frame)) != 0) {
        fprintf(stderr, "Encapsulation failed\n");
        return 1;
    }

    printf("\n=== [PHYSICAL WIRE] SIGNAL TRANSMITTED OVER WIRE ===\n");
    printf("Wire Frame (%zu bytes):\n%s\n\n", strlen(frame), frame);

    // 2. Server Decapsulation
    if (osi_server_decapsulate(frame, recovered, sizeof(recovered)) != 0) {
        fprintf(stderr, "Decapsulation failed\n");
        return 1;
    }

    if (strcmp(msg, recovered) == 0) {
        printf("====================================================\n");
        printf(" [VERIFICATION SUCCESS] Exact Match Recovered!\n");
        printf(" Input    : %s\n", msg);
        printf(" Output   : %s\n", recovered);
        printf("====================================================\n");
    } else {
        printf(" [ERROR] Checksum or decapsulation mismatch!\n");
        return 1;
    }

    return 0;
}
