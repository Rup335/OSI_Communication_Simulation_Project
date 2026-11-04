#include "layers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_layer_log(int layer_num, const char *layer_name, const char *action, const char *data) {
    printf("[net:osi] [ OK ] Layer %d (%-12s): %s\n", layer_num, layer_name, action);
    if (data && strlen(data) > 0) {
        if (strlen(data) > 80) {
            printf("          └─ Payload [%zu bytes]: %.75s...\n", strlen(data), data);
        } else {
            printf("          └─ Payload: %s\n", data);
        }
    }
}

void print_progress_checkpoint(int checkpoint_num, const char *layers_desc) {
    printf("\n  --------------------------------------------------------\n");
    printf("  >>> [PROGRESS CHECKPOINT %d] %s\n", checkpoint_num, layers_desc);
    printf("  --------------------------------------------------------\n\n");
}

// ==================== SEND FUNCTIONS (L7 -> L1) ====================

// Layer 7: Application
int app_send(const char *msg, char *out, size_t out_sz) {
    print_layer_log(7, "Application", "Generating user application payload", msg);
    int ret = snprintf(out, out_sz, "APP_HDR|%s", msg);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 6: Presentation
int present_send(const char *data, char *out, size_t out_sz) {
    print_layer_log(6, "Presentation", "Encoding ASCII data representation", data);
    int ret = snprintf(out, out_sz, "PRS_HDR|%s", data);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 5: Session
int session_send(const char *data, char *out, size_t out_sz) {
    print_layer_log(5, "Session", "Establishing session connection (ID=1001, SYN=1)", data);
    int ret = snprintf(out, out_sz, "SES_HDR|%s", data);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 4: Transport
int transport_send(const char *data, char *out, size_t out_sz) {
    print_layer_log(4, "Transport", "Adding TCP segment header (Src:54321, Dst:8080, Seq:1)", data);
    int ret = snprintf(out, out_sz, "TCP_HDR|%s", data);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 3: Network
int network_send(const char *data, char *out, size_t out_sz) {
    print_layer_log(3, "Network", "Adding IP packet header (127.0.0.1 -> 127.0.0.1, TTL:64)", data);
    int ret = snprintf(out, out_sz, "IP_HDR|%s", data);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 2: Data Link
int datalink_send(const char *data, char *out, size_t out_sz) {
    uint8_t fcs = compute_checksum(data, strlen(data));
    print_layer_log(2, "Data Link", "Framing Ethernet frame with MAC header & FCS checksum", data);
    int ret = snprintf(out, out_sz, "MAC_HDR|%s|FCS:%02X", data, fcs);
    return (ret < 0 || (size_t)ret >= out_sz) ? -1 : 0;
}

// Layer 1: Physical
int physical_send(const char *data, SOCKET sock) {
    uint32_t len = (uint32_t)strlen(data);
    print_layer_log(1, "Physical", "Converting frame to raw wire bitstream", data);

    // Send 4-byte network-order length prefix + raw frame
    uint32_t net_len = htonl(len);
    if (send_all(sock, &net_len, sizeof(net_len)) != 0) {
        fprintf(stderr, "[Physical Error] Failed to send length prefix\n");
        return -1;
    }
    if (send_all(sock, data, len) != 0) {
        fprintf(stderr, "[Physical Error] Failed to send wire bitstream\n");
        return -1;
    }
    printf("[net:osi] [ OK ] Signal successfully transmitted over real TCP socket (%u bytes)\n", len);
    return 0;
}

// Client Encapsulation Flow with 2-Layer Progress Tracking
int osi_client_encapsulate(const char *msg, char *out_frame, size_t out_sz) {
    char b7[MAX_BUFFER], b6[MAX_BUFFER], b5[MAX_BUFFER], b4[MAX_BUFFER], b3[MAX_BUFFER];

    printf("\n=== [CLIENT] OSI 7-LAYER ENCAPSULATION STARTING ===\n");
    printf("Input Message: \"%s\"\n\n", msg);

    // Layer 7
    if (app_send(msg, b7, sizeof(b7)) != 0) return -1;
    // Layer 6
    if (present_send(b7, b6, sizeof(b6)) != 0) return -1;
    // Progress Checkpoint 1: 7 & 6
    print_progress_checkpoint(1, "Layers 7 and 6 completed.");

    // Layer 5
    if (session_send(b6, b5, sizeof(b5)) != 0) return -1;
    // Layer 4
    if (transport_send(b5, b4, sizeof(b4)) != 0) return -1;
    // Progress Checkpoint 2: 5 & 4
    print_progress_checkpoint(2, "Layers 5 and 4 completed.");

    // Layer 3
    if (network_send(b4, b3, sizeof(b3)) != 0) return -1;
    // Layer 2
    if (datalink_send(b3, out_frame, out_sz) != 0) return -1;
    // Progress Checkpoint 3: 3 & 2
    print_progress_checkpoint(3, "Layers 3 and 2 completed.");

    return 0;
}


// ==================== RECEIVE FUNCTIONS (L1 -> L7) ====================

// Layer 1: Physical
int physical_recv(SOCKET sock, char *out, size_t out_sz) {
    uint32_t net_len = 0;
    if (recv_all(sock, &net_len, sizeof(net_len)) != 0) {
        return -1;
    }
    uint32_t len = ntohl(net_len);
    if (len >= out_sz) {
        fprintf(stderr, "[Physical RX Error] Frame exceeds buffer size\n");
        return -1;
    }
    if (recv_all(sock, out, len) != 0) {
        fprintf(stderr, "[Physical RX Error] Failed to read bitstream\n");
        return -1;
    }
    out[len] = '\0';
    print_layer_log(1, "Physical", "Received raw bitstream from wire and reassembled frame", out);
    return 0;
}

// Layer 2: Data Link
int datalink_recv(const char *frame, char *out, size_t out_sz) {
    // Format: MAC_HDR|IP_HDR|...|FCS:XX
    if (strncmp(frame, "MAC_HDR|", 8) != 0) {
        fprintf(stderr, "[DataLink RX Error] Invalid MAC header\n");
        return -1;
    }
    const char *fcs_ptr = strstr(frame, "|FCS:");
    if (!fcs_ptr) {
        fprintf(stderr, "[DataLink RX Error] Missing FCS trailer\n");
        return -1;
    }
    unsigned int rcv_fcs = 0;
    if (sscanf(fcs_ptr + 5, "%02X", &rcv_fcs) != 1) {
        fprintf(stderr, "[DataLink RX Error] Invalid FCS format\n");
        return -1;
    }
    const char *payload_start = frame + 8;
    size_t payload_len = fcs_ptr - payload_start;
    if (payload_len >= out_sz) return -1;

    memcpy(out, payload_start, payload_len);
    out[payload_len] = '\0';

    uint8_t calc_fcs = compute_checksum(out, payload_len);
    if (calc_fcs != (uint8_t)rcv_fcs) {
        fprintf(stderr, "[DataLink RX Error] FCS Checksum mismatch!\n");
        return -1;
    }
    print_layer_log(2, "Data Link", "FCS Checksum Verified. Stripped MAC header and FCS trailer", out);
    return 0;
}

// Helper: Strip PREFIX|
static int strip_prefix(const char *input, const char *prefix, char *out, size_t out_sz) {
    size_t plen = strlen(prefix);
    if (strncmp(input, prefix, plen) != 0 || input[plen] != '|') {
        fprintf(stderr, "[Decapsulation Error] Expected prefix '%s|', got: %.30s\n", prefix, input);
        return -1;
    }
    const char *data_start = input + plen + 1;
    size_t len = strlen(data_start);
    if (len >= out_sz) return -1;

    memcpy(out, data_start, len + 1);
    return 0;
}

// Layer 3: Network
int network_recv(const char *data, char *out, size_t out_sz) {
    if (strip_prefix(data, "IP_HDR", out, out_sz) != 0) return -1;
    print_layer_log(3, "Network", "IP Header validated and stripped", out);
    return 0;
}

// Layer 4: Transport
int transport_recv(const char *data, char *out, size_t out_sz) {
    if (strip_prefix(data, "TCP_HDR", out, out_sz) != 0) return -1;
    print_layer_log(4, "Transport", "TCP Header validated and stripped", out);
    return 0;
}

// Layer 5: Session
int session_recv(const char *data, char *out, size_t out_sz) {
    if (strip_prefix(data, "SES_HDR", out, out_sz) != 0) return -1;
    print_layer_log(5, "Session", "Session connection validated and closed", out);
    return 0;
}

// Layer 6: Presentation
int present_recv(const char *data, char *out, size_t out_sz) {
    if (strip_prefix(data, "PRS_HDR", out, out_sz) != 0) return -1;
    print_layer_log(6, "Presentation", "ASCII format translation validated and stripped", out);
    return 0;
}

// Layer 7: Application
int app_recv(const char *data, char *out, size_t out_sz) {
    if (strip_prefix(data, "APP_HDR", out, out_sz) != 0) return -1;
    print_layer_log(7, "Application", "Extracted original message for user application", out);
    return 0;
}

// Server Decapsulation Flow with 2-Layer Progress Tracking
int osi_server_decapsulate(const char *frame, char *recovered_msg, size_t out_sz) {
    char b3[MAX_BUFFER], b4[MAX_BUFFER], b5[MAX_BUFFER], b6[MAX_BUFFER], b7[MAX_BUFFER];

    printf("\n=== [SERVER] OSI 7-LAYER DECAPSULATION STARTING ===\n\n");

    // Layer 2: Data Link
    if (datalink_recv(frame, b3, sizeof(b3)) != 0) return -1;
    // Progress Checkpoint 1 on RX: Layers 1 & 2
    print_progress_checkpoint(1, "Layers 1 and 2 completed.");

    // Layer 3: Network
    if (network_recv(b3, b4, sizeof(b4)) != 0) return -1;
    // Layer 4: Transport
    if (transport_recv(b4, b5, sizeof(b5)) != 0) return -1;
    // Progress Checkpoint 2 on RX: Layers 3 & 4
    print_progress_checkpoint(2, "Layers 3 and 4 completed.");

    // Layer 5: Session
    if (session_recv(b5, b6, sizeof(b6)) != 0) return -1;
    // Layer 6: Presentation
    if (present_recv(b6, b7, sizeof(b7)) != 0) return -1;
    // Progress Checkpoint 3 on RX: Layers 5 & 6
    print_progress_checkpoint(3, "Layers 5 and 6 completed.");

    // Layer 7: Application
    if (app_recv(b7, recovered_msg, out_sz) != 0) return -1;

    printf("\n>>> [DELIVERY COMPLETE] Original User Message: \"%s\"\n\n", recovered_msg);
    return 0;
}
