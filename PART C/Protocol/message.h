#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../Node/node.h"

#define PAYLOAD_SIZE 492
typedef enum func_id
{
  FUNC_ID_ACK = 1,
  FUNC_ID_NACK,
  FUNC_ID_CONNECT = 4,
  FUNC_ID_DISCOVER = 8,
  FUNC_ID_ROUTE = 16,
  FUNC_ID_SEND = 32,
  FUNC_ID_RELAY = 64
} MESSAGE_function_id;

typedef struct message
{
  uint32_t msg_id;
  uint32_t src_id;
  uint32_t dst_id;
  int32_t trailing_msg;
  uint32_t func_id;
  char payload[PAYLOAD_SIZE];
} message;

bool message_check_format(message *msg);
int32_t send_connect_message(short sock, uint32_t src_node_id);
bool message_parse(Node *node, char *buffer, size_t len, int32_t sfrom_fd);
bool send_message(short sock, int32_t src_id, int32_t dst_id, size_t len, char *data, uint32_t chunk_num);
bool send_route_message(short sock, int32_t src_node_id, int32_t dst_node_id, Route *route);
int64_t send_discover_message(short sock, int32_t src_id, int32_t dst_id, int32_t target_id);
bool send_ack_message(short sock, int32_t src_node_id, int32_t current_node, int32_t payload);
bool send_nack_message(short sock, int32_t src_node_id, int32_t current_node, int32_t payload);