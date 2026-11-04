#ifndef LAYERS_H
#define LAYERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "osi_common.h"

#define MAX_PAYLOAD 2048

// Linux/Ubuntu-style progress logger
void print_layer_log(int layer_num, const char *layer_name, const char *action, const char *data);
void print_progress_checkpoint(int checkpoint_num, const char *layers_desc);

// ==================== SEND FUNCTIONS (L7 -> L1) ====================
int app_send(const char *msg, char *out, size_t out_sz);
int present_send(const char *data, char *out, size_t out_sz);
int session_send(const char *data, char *out, size_t out_sz);
int transport_send(const char *data, char *out, size_t out_sz);
int network_send(const char *data, char *out, size_t out_sz);
int datalink_send(const char *data, char *out, size_t out_sz);
int physical_send(const char *data, SOCKET sock);

// Full encapsulation orchestrator
int osi_client_encapsulate(const char *msg, char *out_frame, size_t out_sz);

// ==================== RECEIVE FUNCTIONS (L1 -> L7) ====================
int physical_recv(SOCKET sock, char *out, size_t out_sz);
int datalink_recv(const char *frame, char *out, size_t out_sz);
int network_recv(const char *data, char *out, size_t out_sz);
int transport_recv(const char *data, char *out, size_t out_sz);
int session_recv(const char *data, char *out, size_t out_sz);
int present_recv(const char *data, char *out, size_t out_sz);
int app_recv(const char *data, char *out, size_t out_sz);

// Full decapsulation orchestrator
int osi_server_decapsulate(const char *frame, char *recovered_msg, size_t out_sz);

// Backward-compatibility aliases
#define Application_Send  app_send
#define Presentation_Send present_send
#define Session_Send      session_send
#define Transport_Send    transport_send
#define Network_Send      network_send
#define DataLink_Send     datalink_send
#define Physical_Send     physical_send

#define Physical_Receive     physical_recv
#define DataLink_Receive     datalink_recv
#define Network_Receive      network_recv
#define Transport_Receive    transport_recv
#define Session_Receive      session_recv
#define Presentation_Receive present_recv
#define Application_Receive  app_recv

#endif // LAYERS_H
