#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct neighbor
{
    int32_t id;
    short connection;
    uint32_t ip_addr;
    uint32_t port;
} Neighbor;

int32_t Neighbor_get_index_by_ip_port(Neighbor *neghibors, int32_t len, int32_t fd);
short Neighbor_get_sock_by_id(Neighbor *nodes, size_t size, int32_t id);
bool Neighbor_exists(Neighbor *nodes, int32_t size, int32_t id);
